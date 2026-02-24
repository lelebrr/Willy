import subprocess
import os

def scan_calls():
    elf_path = r'.pio\build\CYD-2USB\firmware.elf'
    objdump_path = r'C:\Users\leleb\.platformio\packages\toolchain-xtensa-esp-elf\bin\xtensa-esp32-elf-objdump.exe'

    if not os.path.exists(elf_path):
        print(f"ELF not found: {elf_path}")
        return

    target_addr = '40252c1c'
    print(f"Scanning {elf_path} for calls to {target_addr} (__analogReadMilliVolts)...")

    cmd = [objdump_path, '-d', elf_path]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, encoding='utf-8', errors='ignore')

    current_func = ""
    for line in proc.stdout:
        if line.endswith(">:\n"):
            current_func = line.strip()
        if 'call' in line and '402a0650' in line:
            print(f"Found call in {current_func}: {line.strip()}")

    proc.wait()

if __name__ == "__main__":
    scan_calls()
