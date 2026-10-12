import subprocess
result = subprocess.run(['git', 'verify-pack', '-v', '.git/objects/pack/pack-5ae2c7f22f65b8990b7367d765aaef11a85665de.idx'], capture_output=True, text=True)
lines = result.stdout.strip().split('\n')
blobs = []
for line in lines:
    parts = line.split()
    if len(parts) >= 5 and parts[2] == 'blob':
        size = int(parts[1])
        name = parts[4]
        blobs.append((size, name))
blobs.sort(reverse=True)
for size, name in blobs[:20]:
    print(f'{size:>8} {name}')