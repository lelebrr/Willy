# Índice de Ataques BadUSB e BlueDucky

## Descrição

Esta pasta contém scripts de ataque para dispositivos BadUSB e BlueDucky, todos organizados em português com comentários detalhados e nomes descritivos.

## Lista de Ataques Disponíveis

### 1. Willy_Download_Execucao

- **Descrição**: Baixa e executa payloads remotos diretamente na máquina alvo
- **Quando usar**: Para instalar backdoors ou ferramentas de acesso remoto em sistemas Windows
- **Como usar**: Conectar o dispositivo BadUSB e o script será executado automaticamente
- **Pré-requisitos**: Conexão com internet. Modifique a URL do payload conforme necessário
- **Observações**: Script seguro, apenas exibe informações na tela

### 2. Reconhecimento_Linux

- **Descrição**: Coleta completa de informações do sistema Linux, incluindo rede, usuários e privilégios
- **Quando usar**: Para mapear sistemas Linux e identificar pontos de entrada e privilégios de acesso
- **Como usar**: Conectar o dispositivo BadUSB e o script será executado automaticamente
- **Pré-requisitos**: Terminal aberto. Modifique os comandos conforme necessário para diferentes distribuições
- **Observações**: Requer terminal aberto. Modifique os comandos conforme necessário para diferentes distribuições

### 3. Reconhecimento_MacOS

- **Descrição**: Coleta completa de informações do sistema MacOS, incluindo rede, usuários e privilégios
- **Quando usar**: Para mapear sistemas MacOS e identificar pontos de entrada e privilégios de acesso
- **Como usar**: Conectar o dispositivo BadUSB e o script será executado automaticamente
- **Pré-requisitos**: Terminal aberto. Modifique os comandos conforme necessário para diferentes versões do MacOS
- **Observações**: Requer terminal aberto. Modifique os comandos conforme necessário para diferentes versões do MacOS

### 4. Redirecionamento_Sites

- **Descrição**: Redireciona o navegador para sites maliciosos ou de phishing após abrir sites legítimos
- **Quando usar**: Para ataques de phishing e redirecionamento de sites
- **Como usar**: Conectar o dispositivo BadUSB e o script será executado automaticamente
- **Pré-requisitos**: Nenhum
- **Observações**: Modifique os URLs conforme necessário. Pode ser combinado com outros ataques para maior eficácia

### 5. Reconhecimento_Sistema

- **Descrição**: Coleta completa de informações do sistema Windows, incluindo rede, usuários, processos e senhas
- **Quando usar**: Para mapear sistemas Windows e identificar pontos de entrada e privilégios de acesso
- **Como usar**: Conectar o dispositivo BadUSB e o script será executado automaticamente
- **Pré-requisitos**: Privilégios de administrador para acesso completo
- **Observações**: Modifique os comandos conforme necessário

### 6. Reverse_Shell

- **Descrição**: Estabelece uma conexão reversa com o atacante, fornecendo controle total sobre o sistema
- **Quando usar**: Para obter acesso remoto persistente a sistemas comprometidos
- **Como usar**: Conectar o dispositivo BadUSB e o script será executado automaticamente. Configure o LHOST e LPORT antes do uso
- **Pré-requisitos**: Um listener ativo no endereço IP e porta especificados
- **Observações**: Requer um listener ativo no endereço IP e porta especificados. Modifique conforme necessário

## Ataques Pendentes de Transformação

Os seguintes arquivos ainda precisam ser transformados em ataques funcionais:

- Agendador_Tarefas.txt
- Bypass_Firewall.txt
- Caixa_Mensagem.txt
- Captura_Tela.txt
- Capturar_Clipboard.txt
- Capturar_WiFi.txt
- Criar_Usuario_Administrador.txt
- Desativar_Antivirus.txt
- DLL_Hijacking.txt
- DLL_Injection.txt
- Download_e_Execucao.txt
- Escalonamento_Privilegios.txt
- Extrair_Credenciais.txt
- Hack_Webcam.txt
- Hooking.txt
- Injecao_Processo.txt
- Keylogger.txt
- Persistencia_Registry.txt
- Persistencia_Servico.txt
- Persistencia_WMI.txt
- Persistencia.txt
- Ransomware.txt
- Rootkit.txt
- Roubar_Dados.txt
- Scanner_Rede.txt
- Verificar_Privilegios.txt

## Instruções de Uso

### Preparação do Dispositivo

1. Use um dispositivo BadUSB compatível (Arduino, Teensy, etc.)
2. Grave o script Ducky correspondente no dispositivo
3. Teste o dispositivo em um ambiente controlado antes de usar

### Execução dos Ataques

1. Conecte o dispositivo BadUSB na máquina alvo
2. Os scripts serão executados automaticamente
3. Monitore os resultados conforme o tipo de ataque

### Segurança

- **Aviso**: Todos os scripts são para fins educacionais e de teste
- **Responsabilidade**: Use apenas em sistemas que você possui permissão para testar
- **Legalidade**: Consulte as leis locais sobre testes de segurança informática

## Estrutura de Arquivos

```
ataques_badusb/
├── README.md                    # Este arquivo de índice
├── Willy_Download_Execucao.txt  # Ataque de download e execução
├── Reconhecimento_Linux.txt     # Reconhecimento de sistemas Linux
├── Reconhecimento_MacOS.txt     # Reconhecimento de sistemas MacOS
├── Redirecionamento_Sites.txt   # Redirecionamento de sites
├── Reconhecimento_Sistema.txt   # Reconhecimento completo Windows
└── Reverse_Shell.txt            # Conexão reversa avançada
```

## Próximos Passos

1. Transformar todos os arquivos restantes em ataques funcionais
2. Adicionar comentários em português a todos os arquivos
3. Organizar as outras pastas de ataques
4. Criar índice geral de todos os ataques

## Contato

Para dúvidas ou sugestões, consulte a documentação principal do projeto.
