# U2F via USB (`src/modules/others/u2f.cpp`)

Transforma o aparelho em chave de segurança FIDO U2F.

## Acesso

**Outros → BadUSB & HID → U2F USB** (`u2f_setup()`, builds com `USB_as_HID`). Aguarda o host montar o HID (timeout de 4 s) e processa as requisições U2F; saia com ESC.

## Notas

- Exige placa com USB HID nativo (`USB_as_HID`); sem ele a entrada não aparece.
- O loop bombeia `InputHandler()` para manter os botões responsivos durante o atendimento.
