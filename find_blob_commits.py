import subprocess
# Find commits that contain the large blob
target = '23abddc9292d3ce93031526168c8885b0d923aef'
r = subprocess.run(['git', 'rev-list', '--all', '--', target], 
                   capture_output=True, text=True, cwd='F:\\Willy')
print('stdout:', r.stdout[:2000])
print('stderr:', r.stderr[:2000])
print('returncode:', r.returncode)