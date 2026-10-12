import subprocess

# Get all objects with their file paths
result = subprocess.run(['git', 'log', '--all', '--diff-filter=A', '--name-only', '--format=%H'], capture_output=True, text=True)
print(result.stdout[:5000])