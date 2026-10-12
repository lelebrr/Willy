import subprocess

# Check the large blob from the big pack
result = subprocess.run(['git', 'verify-pack', '-v', '.git/objects/pack/pack-2bd213dfc6b6545dce5df9998e4e710ba0efd5a5.idx'], capture_output=True)
lines = result.stdout.decode().strip().split('\n')

# Find the specific large blob
target_name = '23abddc9292d3ce93031526168c8885b0d923aef'
for line in lines:
    parts = line.split()
    if len(parts) >= 5 and parts[0] == target_name:
        print(f'Found: {line}')
        # parts format: name type size offset [previous]
        print(f'Name: {parts[0]}')
        print(f'Type: {parts[1]}')
        print(f'Size: {parts[2]}')
        print(f'Offset: {parts[3]}')
        if len(parts) > 5:
            print(f'Previous: {parts[5]}')
        break

# Also try to find what file this blob maps to by checking all packs
print('\n--- Checking all packs for this blob ---')
pack_dir = '.git/objects/pack'
import os
pack_files = [f for f in os.listdir(pack_dir) if f.startswith('pack-') and f.endswith('.pack')]

for pf in pack_files:
    idx_file = pf.replace('.pack', '.idx')
    result2 = subprocess.run(['git', 'ls-tree', '-r', f'--unfiltered', '--no-merges', '@'], capture_output=True, text=True)
    # Just check if the object name appears in any tree
    if target_name in result2.stdout:
        print(f'Found in {pf}')