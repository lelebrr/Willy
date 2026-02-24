import sys

def analyze(search_str=None):
    try:
        with open('symbols.txt', 'r', encoding='utf-16') as f:
            symbols = f.readlines()
    except:
        try:
            with open('symbols.txt', 'r', encoding='utf-8') as f:
                symbols = f.readlines()
        except:
            with open('symbols.txt', 'rb') as f:
                symbols = [s.decode(errors='ignore') for s in f.readlines()]

    print('=== ADC Symbols ===')
    for s in symbols:
        if 'adc' in s.lower():
            print(s.strip())

    legacy = ['adc1_config_width', 'adc1_config_channel_atten', 'adc1_get_raw', 'adc2_vref_to_gpio', 'adc_vref_to_gpio', 'adc_power_acquire', 'adc_gpio_init']
    print('\n=== Legacy ADC Symbols Found ===')
    for s in symbols:
        for l in legacy:
            if l in s:
                print(s.strip())

    if search_str:
        print(f'\n=== Searching for "{search_str}" ===')
        for s in symbols:
            if search_str.lower() in s.lower():
                print(s.strip())

    targets = [0x4025f27a, 0x402a062b]

    parsed = []
    for s in symbols:
        parts = s.split()
        if len(parts) >= 2:
            try:
                # Handle cases like "addr size T name" or "addr T name"
                addr = int(parts[0], 16)
                parsed.append((addr, s.strip()))
            except ValueError:
                continue

    parsed.sort()

    for target in targets:
        print(f'\n=== Symbol for 0x{target:x} ===')
        found = False
        for i in range(len(parsed)):
            if parsed[i][0] > target:
                if i > 0:
                    print(f'Target 0x{target:x} is inside/after: {parsed[i-1][1]}')
                    print(f'Next symbol is: {parsed[i][1]}')
                else:
                    print(f'Target 0x{target:x} is BEFORE first symbol: {parsed[i][1]}')
                found = True
                break
        if not found:
            print(f'Target 0x{target:x} is AFTER last symbol: {parsed[-1][1]}')

if __name__ == "__main__":
    search = sys.argv[1] if len(sys.argv) > 1 else None
    analyze(search)
