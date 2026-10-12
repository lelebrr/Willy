import subprocess, os

pack_dir = '.git/objects/pack'
pack_files = [f for f in os.listdir(pack_dir) if f.startswith('pack-') and f.endswith('.pack')]

for pf in pack_files:
    idx_file = pf.replace('.pack', '.idx')
    print(f'\\n=== Checking {pf} ===')
    try:
        result = subprocess.run(['git', 'verify-pack', '-v', os.path.join(pack_dir, idx_file)], capture_output=True)
        lines = result.stdout.decode().strip().split('\n')
        blobs = []
        for line in lines:
            parts = line.split()
            if len(parts) >= 3 and parts[1] == 'blob':
                size = int(parts[2])
                name = parts[0]
                blobs.append((size, name))
        blobs.sort(reverse=True)
        print(f'Total blobs: {len(blobs)}')
        print('Top 5 largest blobs:')
        for b in blobs[:5]:
            print(f'  {b[0]:>8} {b[1]}')
    except Exception as e:
        print(f'Error: {e}')