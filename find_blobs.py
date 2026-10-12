import subprocess
r = subprocess.run(['git', 'verify-pack', '-v', '.git/objects/pack/pack-5ae2c7f22f65b8990b7367d765aaef11a85665de.idx'], capture_output=True)
lines = r.stdout.decode().strip().split('\n')
blobs = []
for line in lines:
    parts = line.split()
    if len(parts) >= 3 and parts[1] == 'blob':
        size = int(parts[2])
        name = parts[0]
        blobs.append((size, name))
blobs.sort(reverse=True)
print('Top 20 largest blobs:')
for b in blobs[:20]:
    print(f'{b[0]:>8} {b[1]}')
print(f'\nTotal blobs: {len(blobs)}')