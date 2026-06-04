#include "badusb_commands.h"
#include "core/sd_functions.h"
#include "helpers.h"
#include "modules/badusb_ble/ducky_typer.h"

uint32_t badusbFileCallback(cmd *c) {
#ifndef LITE_VERSION
    // badusb run_from_file HelloWorld.txt

    Command cmd(c);

    Argument arg = cmd.getArgument("filepath");
    String filepath = arg.getValue();
    filepath.trim();

    if (filepath.indexOf(".txt") == -1) {
        serialDevice->println("Invalid filename");
        return false;
    }
    if (!filepath.startsWith("/")) filepath = "/" + filepath;

    FS *fs;
    if (!getFsStorage(fs)) return false;

    if (!(*fs).exists(filepath)) {
        serialDevice->println("File does not exist");
        return false;
    }

#ifdef USB_as_HID
    ducky_startKb(hid_usb, false);
    key_input(*fs, filepath, hid_usb);
    ducky_stopKb(hid_usb, false);

    return true;
#else
    return false;
#endif
#else
    return false;
#endif
}


class StringStream : public Stream {
public:
    StringStream(const String &s) : str(s), pos(0) {}
    StringStream(const char *c) : str(c), pos(0) {}
    virtual size_t write(uint8_t) { return 0; }
    virtual int available() { return str.length() - pos; }
    virtual int read() { return pos < str.length() ? str[pos++] : -1; }
    virtual int peek() { return pos < str.length() ? str[pos] : -1; }
    virtual void flush() {}
private:
    String str;
    unsigned int pos;
};

uint32_t badusbBufferCallback(cmd *c) {
#ifndef LITE_VERSION

    char *txt = _readFileFromSerial();
    if (!txt) return false;

#ifdef USB_as_HID
    ducky_startKb(hid_usb, false);

    StringStream stream(txt);
    key_input(&stream, "buffer", hid_usb);

    delete hid_usb;
    hid_usb = nullptr;

    free(txt);
    return true;
#else
    free(txt);
    return false;
#endif
#else
    return false;
#endif
}

void createBadUsbCommands(SimpleCLI *cli) {
#ifndef LITE_VERSION
    Command badusbCmd = cli->addCompositeCmd("bu,badusb");

    Command fileCmd = badusbCmd.addCommand("run_from_file", badusbFileCallback);
    fileCmd.addPosArg("filepath");

    Command bufferCmd = badusbCmd.addCommand("run_from_buffer", badusbBufferCallback);
#endif
}
