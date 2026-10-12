import subprocess
r = subprocess.run(['git', 'verify-pack', '-v', '.git/objects/pack/pack-5ae2c7f22f65b8990b7367d765aaef11a85665de.idx'], capture_output=True)
lines = r.stdout.decode().strip().split('\n')
# Check first few lines format
for i in range(3):
    parts = lines[i].split()
    print(f'Line {i}: {parts}')
    if len(parts) >= 3:
        print(f'  parts[1] (type): "{parts[1]}"')
        print(f'  parts[2] (size): "{parts[2]}"')