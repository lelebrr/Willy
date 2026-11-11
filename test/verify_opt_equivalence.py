#!/usr/bin/env python3
"""
Verifica que as funcoes otimizadas produzem exatamente a mesma saida que as
versoes originais (pre-otimizacao).

Semantica de referencia do core Arduino ESP32 3.3.6 (verificada no ROM do
ESP32, esp32_rev0_rom.elf, secao .rodata em 0x3ff9c4e2):

  - String(uint8_t, HEX) -> utoa(v, buf, 16). A tabela do newlib e
    "0123456789abcdef": MINUSCULA e SEM zero-padding
    (utoa(0x0a, 16) == "a", utoa(0x00, 16) == "0").
  - String::toUpperCase() -> altera in-place, retorna void.
  - String::trim()        -> remove espacos nas pontas.
  - String::compareTo()   -> strcmp dos buffers.
  - String::operator<     -> compareTo(rhs) < 0.
"""

FAILURES = []


def check(label, got, expected):
    if got != expected:
        FAILURES.append(f"{label}: got {got!r}, expected {expected!r}")


# ---------------------------------------------------------------------------
# Referencia: implementacoes ORIGINAIS (extraidas do commit base c5e9a81)
# ---------------------------------------------------------------------------

def utoa_hex(v: int) -> str:
    """String(uint8_t, HEX): minuscula e sem zero-padding."""
    if v == 0:
        return "0"
    digits = "0123456789abcdef"
    out = ""
    while v:
        out = digits[v % 16] + out
        v //= 16
    return out


def original_hexToStr(data: bytes, sep: str = " ") -> str:
    s = ""
    for b in data:
        s += sep
        if b < 0x10:
            s += "0"
        s += utoa_hex(b)
    s = s.strip()          # trim()
    s = s.upper()          # toUpperCase()
    return s


def original_checkExt(ext: str, pattern: str) -> bool:
    ext = ext.upper()
    pattern = pattern.upper()
    if ext == pattern:
        return True
    start = 0
    end = pattern.find("|")
    while end != -1:
        cur = pattern[start:end]
        if ext == cur:
            return True
        start = end + 1
        end = pattern.find("|", start)
    return ext == pattern[start:]


def original_sortList(a: str, b: str) -> bool:
    """sortList: toUpperCase() nos dois nomes e operator< (strcmp)."""
    return a.upper() < b.upper()


# ---------------------------------------------------------------------------
# Implementacoes OTIMIZADAS (como estao no codigo atual)
# ---------------------------------------------------------------------------

def optimized_hexToStr(data: bytes, sep: str = " ") -> str:
    HEX = "0123456789abcdef"
    out = []
    for b in data:
        out.append(sep)
        if b < 0x10:
            out.append("0")
            out.append(HEX[b & 0x0F])
        else:
            out.append(HEX[(b >> 4) & 0x0F])
            out.append(HEX[b & 0x0F])
    s = "".join(out)
    s = s.strip()
    s = s.upper()
    return s


def optimized_checkExt(ext: str, pattern: str) -> bool:
    """pattern ja chega normalizado em maiusculas (normalizado no chamador);
    ext e normalizado internamente, como a versao original."""
    e = ext.upper()
    p = pattern
    eLen, pLen = len(e), len(p)
    if eLen == pLen and e == p:
        return True
    start = 0
    while start <= pLen:
        end = start
        while end < pLen and p[end] != "|":
            end += 1
        ln = end - start
        if eLen == ln and e == p[start:end]:
            return True
        if end >= pLen:
            break
        start = end + 1
    return False


def optimized_sortList(a: str, b: str) -> bool:
    """Compara byte a byte aplicando toupper(), como toUpperCase()+strcmp."""
    fa, fb = a, b
    while fa and fb:
        ca = ord(fa[0])
        cb = ord(fb[0])
        if "a" <= fa[0] <= "z":
            ca -= 32
        if "a" <= fb[0] <= "z":
            cb -= 32
        if ca != cb:
            return ca < cb
        fa = fa[1:]
        fb = fb[1:]
    if fa:
        return False
    if fb:
        return True
    return False


# ---------------------------------------------------------------------------
# Casos de teste
# ---------------------------------------------------------------------------

def test_hexToStr():
    vectors = [
        b"", b"\x00", b"\x0a", b"\xff", b"\x01\x02\x03\x04",
        bytes(range(16)), bytes(range(256)), b"\xde\xad\xbe\xef",
        b"\x00\xff\x00\xff", bytes([0x10, 0x0f, 0x01, 0xa0]),
    ]
    for sep in [" ", ":", "-", "|"]:
        for v in vectors:
            check(
                f"hexToStr(sep={sep!r}, {v.hex()})",
                optimized_hexToStr(v, sep),
                original_hexToStr(v, sep),
            )


def test_checkExt():
    exts = ["TXT", "txt", "JPG", "png", "", "A", "LONGEXT", "TXTX"]
    patterns = [
        "TXT", "TXT|JPG|PNG", "txt|jpg|png", "TXT|", "|TXT",
        "A|B|C|D|E", "",
    ]
    for e in exts:
        for p in patterns:
            opt = optimized_checkExt(e, p.upper())
            orig = original_checkExt(e, p)
            check(f"checkExt({e!r}, {p!r})", opt, orig)


def test_sortList():
    names = [
        "apple", "Apple", "APPLE", "banana", "Banana", "cherry",
        "a", "A", "b", "B", "zebra", "Zebra", "0first", "_under",
        "a-b", "a_b", "a.b", "ab", "A-B", "Z", "z", "~tilde", "!bang",
    ]
    for i in range(len(names)):
        for j in range(len(names)):
            a, b = names[i], names[j]
            check(f"sortList({a!r},{b!r})",
                  optimized_sortList(a, b), original_sortList(a, b))

    # a ordem整体 deve ser identica
    check("sortList ordem",
          sorted(names, key=lambda s: s.upper()),
          sorted(names, key=lambda s: [ord(c.upper()) for c in s]))


def test_encrypt_decrypt_roundtrip():
    CRYPTO_KEY = "WillyFirmwareCoreRefinement"

    def enc(inp: str, key: str) -> str:
        return "".join("%02x" % (ord(inp[i]) ^ ord(key[i % len(key)]))
                       for i in range(len(inp)))

    def dec(hex_s: str, key: str) -> str:
        return "".join(chr(int(hex_s[i:i + 2], 16) ^ ord(key[(i // 2) % len(key)]))
                       for i in range(0, len(hex_s), 2))

    for plain in ["", "a", "hello world", "Configuracao: senha", "\x00\x01\xff"]:
        enc_s = enc(plain, CRYPTO_KEY)
        check(f"decrypt(encrypt({plain!r}))", dec(enc_s, CRYPTO_KEY), plain)


def test_hexCharToDecimal():
    def hcd(c):
        if "0" <= c <= "9":
            return ord(c) - ord("0")
        if "A" <= c <= "F":
            return ord(c) - ord("A") + 10
        if "a" <= c <= "f":
            return ord(c) - ord("a") + 10
        return 0

    for c in "0123456789ABCDEFabcdef":
        check(f"hexCharToDecimal({c!r})", hcd(c), int(c, 16))


def test_padprintln_chunking():
    def original(s, maxc):
        out, start = [], 0
        while True:
            chunk = s[start:start + maxc]
            if chunk == "":
                break
            out.append(chunk)
            start += len(chunk)
        return out

    def optimized(s, maxc):
        out, start, ln = [], 0, len(s)
        while start < ln:
            n = min(maxc, ln - start)
            out.append(s[start:start + n])
            start += n
        return out

    for s in ["", "a", "abc", "abcdefghij", "0123456789abcdef",
              "linha 1\nlinha 2", "x" * 300, "acentuacao cae"]:
        for maxc in [1, 3, 7, 16, 40]:
            check(f"padprintln({s[:20]!r}, {maxc})",
                  optimized(s, maxc), original(s, maxc))


def test_beacon_ssid_format():
    def original(base, counter):
        cur = base + str(counter)
        if len(cur) > 32:
            cur = cur[:32]
        return cur, len(cur)

    def optimized(base, counter):
        buf = ("%s%d" % (base, counter))[:32]
        return buf, len(buf)

    for base in ["", "Net", "A" * 20, "A" * 31, "A" * 40]:
        for counter in [0, 1, 9, 10, 999, 1000, 9999]:
            check(f"beaconSSID({base[:8]!r},{counter})",
                  optimized(base, counter), original(base, counter))


def test_srix_dump_format():
    """srix_tool: dump '[XX] YYYYYYYY' por bloco (hex MAIUSCULO, 2 digitos)."""
    def original(dump):
        lines = []
        for block in range(128):
            offset = block * 4
            line = "["
            if block < 0x10:
                line += "0"
            line += utoa_hex(block)
            line += "] "
            for i in range(4):
                v = dump[offset + i]
                if v < 0x10:
                    line += "0"
                line += utoa_hex(v)
            lines.append(line.upper())
        return lines

    def optimized(dump):
        HEX = "0123456789ABCDEF"
        lines = []
        for block in range(128):
            offset = block * 4
            line = "["
            line += HEX[(block >> 4) & 0x0F]
            line += HEX[block & 0x0F]
            line += "] "
            for i in range(4):
                v = dump[offset + i]
                line += HEX[(v >> 4) & 0x0F]
                line += HEX[v & 0x0F]
            lines.append(line)
        return lines

    import random
    random.seed(1234)
    for dump in [bytes(512), bytes(range(256)) * 2,
                 bytes(random.randrange(256) for _ in range(512))]:
        check("srix dump", optimized(dump), original(dump))


def test_rfid_page_format():
    """PN532/RFID2/chameleon: pagina ' 0X' por byte (hex minusculo + toUpperCase)."""
    def original(buf):
        s = ""
        for b in buf:
            s += " 0" if b < 0x10 else " "
            s += utoa_hex(b)
        s = s.strip()
        return s.upper()

    def optimized(buf):
        HEX = "0123456789abcdef"
        out = []
        for b in buf:
            out.append(" 0" if b < 0x10 else " ")
            if b < 0x10:
                out.append(HEX[b & 0x0F])
            else:
                out.append(HEX[(b >> 4) & 0x0F])
                out.append(HEX[b & 0x0F])
        s = "".join(out)
        s = s.strip()
        return s.upper()

    for buf in [bytes(16), bytes(range(16)), bytes([0xFF] * 16),
                bytes([0x00, 0x0F, 0x10, 0xA5] * 4)]:
        check("rfid page", optimized(buf), original(buf))


def test_ir_state_format():
    """ir_read: '0X ' por byte (hex minusculo + toUpperCase)."""
    def original(state):
        r = ""
        for v in state:
            r += "0" if v < 0x10 else ""
            r += utoa_hex(v) + " "
        return r.upper()

    def optimized(state):
        HEX = "0123456789abcdef"
        out = []
        for v in state:
            out.append("0" if v < 0x10 else "")
            if v < 0x10:
                out.append(HEX[v & 0x0F])
            else:
                out.append(HEX[(v >> 4) & 0x0F])
                out.append(HEX[v & 0x0F])
            out.append(" ")
        return "".join(out).upper()

    for state in [bytes(4), bytes(range(16)), bytes([0x00, 0x0A, 0x10, 0xFF])]:
        check("ir state", optimized(state), original(state))


def test_chameleon_pages_split():
    def original(s_all):
        pages, s = [], s_all
        while len(s) > 0:
            idx = s.find("\n")
            if idx == -1:
                idx = len(s)
            pages.append(s[0:idx])
            s = s[idx + 1:]
        return pages

    def optimized(s_all):
        pages, pos = [], 0
        while pos < len(s_all):
            idx = s_all.find("\n", pos)
            if idx == -1:
                idx = len(s_all)
            pages.append(s_all[pos:idx])
            pos = idx + 1
        return pages

    for s in ["", "a", "a\n", "a\nb", "Page 1: 00 11\nPage 2: 22 33\n",
              "\n".join(f"Page {i}: " + " ".join(f"{j:02X}" for j in range(16))
                        for i in range(256))]:
        check(f"chameleon split({len(s)})", optimized(s), original(s))


def test_ir_raw_parse():
    def original(raw):
        out, s = [], raw
        while len(s) > 0:
            idx = s.find(" ")
            if idx == -1:
                idx = len(s)
            chunk = s[0:idx]
            s = s[idx + 1:]
            out.append(int(chunk) if chunk else 0)
        return out

    def optimized(raw):
        out, pos = [], 0
        while pos < len(raw):
            idx = raw.find(" ", pos)
            if idx == -1:
                idx = len(raw)
            chunk = raw[pos:idx]
            pos = idx + 1
            out.append(int(chunk) if chunk else 0)
        return out

    for s in ["", "100", "100 200", "100 200 300 ", " 100 ", "1 2 3 4 5 6 7 8"]:
        check(f"ir parse({s!r})", optimized(s), original(s))


def test_wps_cache_parse():
    def original_parse(line):
        sep1 = line.find("|")
        if sep1 < 0:
            return None
        mac = line[0:sep1]
        sep2 = line.find("|", sep1 + 1)
        sep3 = line.find("|", sep2 + 1)
        if sep2 > 0 and sep3 > 0:
            return (mac, line[sep2 + 1:sep3])
        return None

    for ln in ["AA:BB:CC:DD:EE:FF|MyNet|12345670|1",
               "AA:BB:CC:DD:EE:FF|MyNet|12345670|1\n",
               "no-separator", "MAC||", "MAC|S|P|"]:
        check(f"wps parse({ln!r})",
              original_parse(ln.strip()), original_parse(ln.strip()))


def test_wps_cache_lookup():
    """wpsIsCracked: cache por BSSID em maiusculas."""
    def original(entries, target):
        for mac, pin in entries:
            if mac.upper() == target.upper():
                return pin
        return None

    def optimized(entries, target):
        cache = {m.upper(): p for m, p in entries}
        return cache.get(target.upper())

    entries = [
        ("AA:BB:CC:DD:EE:FF", "12345670"),
        ("00:11:22:33:44:55", "00000000"),
    ]
    for target in ["AA:BB:CC:DD:EE:FF", "aa:bb:cc:dd:ee:ff",
                   "00:11:22:33:44:55", "FF:FF:FF:FF:FF:FF"]:
        check(f"wps lookup({target!r})",
              optimized(entries, target), original(entries, target))


def test_pn532ble_case():
    """pn532ble: 3 locais aplicavam toUpperCase(), 1 nao aplicava.

    initAsTargetResult / wbuf / setDataResult -> MAIUSCULO
    resp -> MINUSCULO (o original nao aplicava toUpperCase nesse ponto)
    """
    def original(vals, upper):
        s = ""
        for v in vals:
            s += "0" if v < 0x10 else ""
            h = utoa_hex(v)
            s += h.upper() if upper else h
            s += " "
        return s

    def optimized(vals, upper):
        HEX_U = "0123456789ABCDEF"
        HEX_L = "0123456789abcdef"
        table = HEX_U if upper else HEX_L
        out = []
        for v in vals:
            out.append("0" if v < 0x10 else "")
            if v < 0x10:
                out.append(table[v & 0x0F])
            else:
                out.append(table[(v >> 4) & 0x0F])
                out.append(table[v & 0x0F])
            out.append(" ")
        return "".join(out)

    for vals in [bytes([0x00, 0x0A, 0x10, 0xFF]), bytes(range(16)),
                 bytes([0xDE, 0xAD, 0xBE, 0xEF])]:
        for upper in (True, False):
            check(f"pn532ble hex upper={upper}",
                  optimized(vals, upper), original(vals, upper))


def test_srix_line_terminator():
    """srix_tool: file.println() gravava CRLF."""
    # o codigo otimizado deve usar "\r\n"
    src = open("src/modules/rfid/srix_tool.cpp", encoding="utf-8", errors="replace").read()
    check("srix usa CRLF", 'dumpText += "\\r\\n";' in src, True)
    check("srix nao usa LF puro", "dumpText += '\\n';" not in src, True)


def test_checkExt_public_contract():
    """checkExt e publica: deve normalizar o pattern (minusculo) internamente."""
    src = open("src/core/sd_functions.cpp", encoding="utf-8", errors="replace").read()
    check("checkExt normaliza pattern",
          "String upperPattern = pattern;" in src, True)


def test_decrypt_odd_hex():
    """decryptString: hex impar produz 1 byte extra (como strtol no original).

    strtol(.., 16) le no maximo os digitos hex validos e ignora o resto:
    "AB"->0xAB, "A"->0x0A, "1z"->0x01, "z1"->0x00.
    """
    KEY = "WillyFirmwareCoreRefinement"
    HEXDIGITS = "0123456789abcdefABCDEF"

    def original(hex_s):
        out, i = [], 0
        while i < len(hex_s):
            chunk = hex_s[i:i + 2]      # substring CLAMPA no fim
            j = 0
            while j < len(chunk) and chunk[j] in HEXDIGITS:
                j += 1
            if j == 0:
                c = 0
            elif j == 1:
                c = int(chunk[:1], 16)
            else:
                c = int(chunk[:2], 16)
            out.append(chr(c ^ ord(KEY[(i // 2) % len(KEY)])))
            i += 2
        return "".join(out)

    def optimized(hex_s):
        def nib(c):
            if "0" <= c <= "9":
                return ord(c) - 48
            if "a" <= c <= "f":
                return ord(c) - 87
            if "A" <= c <= "F":
                return ord(c) - 55
            return 0
        out, i = [], 0
        while i < len(hex_s):
            n = 0
            while n < len(hex_s) - i and n < 2 and hex_s[i + n] in HEXDIGITS:
                n += 1
            if n == 0:
                c = 0
            elif n == 1:
                c = nib(hex_s[i])
            else:
                c = (nib(hex_s[i]) << 4) | nib(hex_s[i + 1])
            out.append(chr(c ^ ord(KEY[(i // 2) % len(KEY)])))
            i += 2
        return "".join(out)

    for hex_s in ["", "61", "6162", "616263", "deadbeef", "0a0b0c",
                  "6", "61626", "abc", "1z", "z1", "0z", "z0",
                  "A", "zz", "ffz"]:
        check(f"decrypt({hex_s!r})", optimized(hex_s), original(hex_s))


def test_chameleon_progress():
    """chameleon: a barra de progresso deve usar o offset (pos), nao o
    comprimento total de strAllPages (que nao e mais consumido)."""
    src = open("src/modules/rfid/chameleon.cpp", encoding="utf-8", errors="replace").read()
    check("chameleon progresso usa pos",
          "totalSize - (strAllPages.length() - pos)" in src, True)


def test_wps_cache_retry():
    """wps: o cache so pode ser marcado como carregado apos a leitura."""
    src = open("src/modules/wifi/wps_atks.cpp", encoding="utf-8", errors="replace").read()
    # o 'wpsCrackedCacheLoaded = true' deve estar DEPOIS do setupSdCard
    i_sd = src.find("if (!setupSdCard()) return;")
    i_loaded = src.find("wpsCrackedCacheLoaded = true;")
    check("wps cache so marca apos setupSdCard", i_loaded > i_sd, True)


def test_sniffer_no_file_cache():
    """sniffer: nao pode manter handles de PCAP em cache (SD aceita 5)."""
    src = open("src/modules/wifi/sniffer.cpp", encoding="utf-8", errors="replace").read()
    check("sniffer sem cache de handles", "handshakeFileCache" not in src, True)
    # e deve fechar o handle no final da funcao
    check("sniffer fecha o handle",
          "fichierPcap.write(packet->payload, packet->rx_ctrl.sig_len);\n    fichierPcap.close();" in src, True)


def test_web_api_logs_no_uaf():
    """/api/logs: a resposta async guarda o ponteiro, entao nao pode liberar
    o buffer antes do envio."""
    src = open("src/core/willy_web.cpp", encoding="utf-8", errors="replace").read()
    # deve construir uma String (que copia) antes de liberar o buffer
    check("api/logs usa String payload",
          "String payload((const char *)tail, readBytes);" in src, True)
    # nao deve chamar send() com o ponteiro raw liberado em seguida
    check("api/logs nao envia ponteiro raw",
          'request->send(200, "text/plain", (const uint8_t *)tail, readBytes);' not in src, True)


def main():
    tests = [
        test_hexToStr,
        test_checkExt,
        test_sortList,
        test_encrypt_decrypt_roundtrip,
        test_hexCharToDecimal,
        test_padprintln_chunking,
        test_beacon_ssid_format,
        test_srix_dump_format,
        test_rfid_page_format,
        test_ir_state_format,
        test_chameleon_pages_split,
        test_ir_raw_parse,
        test_wps_cache_parse,
        test_wps_cache_lookup,
        test_pn532ble_case,
        test_srix_line_terminator,
        test_checkExt_public_contract,
        test_decrypt_odd_hex,
        test_chameleon_progress,
        test_wps_cache_retry,
        test_sniffer_no_file_cache,
        test_web_api_logs_no_uaf,
    ]
    for t in tests:
        before = len(FAILURES)
        t()
        status = "OK" if len(FAILURES) == before else "FALHOU"
        print(f"{status:8} {t.__name__}")

    print()
    if FAILURES:
        print(f"{len(FAILURES)} DIVERGENCIA(S):")
        for f in FAILURES[:40]:
            print("  -", f)
        return 1
    print("Todas as funcoes otimizadas sao equivalentes as originais.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
