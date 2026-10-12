import subprocess
target = '23abddc9292d3ce93031526168c8885b0d923aef'
# Check if blob is in any tree
r = subprocess.run(['git', 'rev-list', '--all', '--objects'], capture_output=True, text=True, cwd='F:\\Willy')
lines = r.stdout.strip().split('\n')
for line in lines:
    parts = line.split()
    if len(parts) >= 2 and parts[1] == target:
        print(f'Found in: {line}')
        break
else:
    print('Blob not found in any commit tree')