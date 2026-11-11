import datetime
import os

def generate_build_header():
    try:
        year = datetime.datetime.now().year
    except:
        year = 2026

    header_content = f'#pragma once\n#define CURRENT_YEAR {year}\n'

    try:
        output_path = os.path.join("include", "current_year.h")
        os.makedirs("include", exist_ok=True)
        with open(output_path, "w") as f:
            f.write(header_content)
    except Exception as e:
        print(f"Warning: Could not create current_year.h - {e}")

def before_build(source, target, env):
    generate_build_header()

# Hook for PlatformIO
def on_pre_build(env):
    before_build(None, None, env)

# Register the pre-build hook
try:
    Import("env")
    on_pre_build(env)
except Exception as e:
    print(f"Warning: hook failed - {e}")
    generate_build_header()
