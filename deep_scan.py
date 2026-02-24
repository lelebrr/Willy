import subprocess
import os
import re

def analyze_constructors():
    elf_path = r'.pio\build\CYD-2USB\firmware.elf'
    objdump_path = r'C:\Users\leleb\.platformio\packages\toolchain-xtensa-esp-elf\bin\xtensa-esp32-elf-objdump.exe'
    addr2line_path = r'C:\Users\leleb\.platformio\packages\toolchain-xtensa-esp-elf\bin\xtensa-esp32-elf-addr2line.exe'

    # List of constructors found earlier
    ctors = [
        0x400e18dc, 0x400e2380, 0x400e24dc, 0x400e2bf4, 0x400e4090, 0x400e4a14, 0x400f6ac0, 0x401079a0,
        0x40109fb4, 0x40112350, 0x40113fe4, 0x40115efc, 0x4011e21c, 0x4011e380, 0x40124138, 0x40124434,
        0x4012562c, 0x4012abd0, 0x40154380, 0x4015e6b8, 0x4016de14, 0x40174b54, 0x40174b7c, 0x40174c94,
        0x40175018, 0x4017702c, 0x40177ea8, 0x40179ebc, 0x4017a4f0, 0x4017f624, 0x40182954, 0x40185a30,
        0x4019f0a0, 0x401a3ef4, 0x401b8338, 0x401c1628, 0x401c1e98, 0x401c520c, 0x401c6558, 0x401c7a0c,
        0x401cd624, 0x401db6b8, 0x401dd454, 0x401e3ca0, 0x401e4584, 0x401edb8c, 0x401f1b58, 0x401f1ba8,
        0x401f540c, 0x401f743c, 0x401fa40c, 0x401fcdf4, 0x40204270, 0x4020c310, 0x40218f00, 0x402211f4,
        0x402336b8, 0x402350a0, 0x4023974c, 0x4023afac, 0x4023e10c, 0x402437e4, 0x4025020c, 0x40250718,
        0x40250e00, 0x402526d0, 0x4025d958, 0x40385da8, 0x40385d1c, 0x400935a4, 0x4029b17c, 0x402a05d8,
        0x402a0604, 0x402be154, 0x4030be1c, 0x4030c714, 0x40315504, 0x4031cb9c, 0x403250b4, 0x40325a10, 0x4032b860
    ]

    # Target addresses for ADC functions
    adc_targets = {
        '402a0650': 'adc_oneshot_new_unit',
        '40252b98': '__analogRead',
        '40252c1c': '__analogReadMilliVolts',
        '402a0604': 'check_adc_oneshot_driver_conflict'
    }

    print(f"Scanning {len(ctors)} global constructors for ADC calls...")

    for ctor in ctors:
        # Disassemble the constructor (limit to a reasonable size, say 512 bytes)
        cmd = [objdump_path, '-d', '--start-address=' + hex(ctor), '--stop-address=' + hex(ctor + 512), elf_path]
        try:
            output = subprocess.check_output(cmd, text=True, encoding='utf-8', errors='ignore')
        except:
            continue

        # Parse the disassembled output
        for line in output.splitlines():
            # Check for direct calls to our target addresses
            for target_addr, target_name in adc_targets.items():
                if target_addr in line and ('call' in line or 'j' in line):
                    # Found a call!
                    # Get the constructor name
                    name_cmd = [addr2line_path, '-fCi', '-e', elf_path, hex(ctor)]
                    name = subprocess.check_output(name_cmd, text=True).splitlines()[0]
                    print(f"\n[CRITICAL] Constructor {hex(ctor)} ({name}) calls {target_name} ({target_addr})!")
                    print(f"Line: {line.strip()}")

if __name__ == "__main__":
    analyze_constructors()
