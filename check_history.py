import subprocess
# Check git log for the large file
r = subprocess.run(['git', 'log', '--all', '--oneline', '--', '.platformio/cache/downloads/e8f9ab15eb1752338cf3e5486bab0a7503c5c36e'], 
                   capture_output=True, text=True, cwd='F:\\Willy')
print('stdout:', r.stdout[:2000])
print('stderr:', r.stderr[:2000])