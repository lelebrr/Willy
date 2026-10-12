Import("env")

# This script separates C and C++ flags to avoid warnings like:
# "cc1.exe: warning: command-line option '-Wno-volatile' is valid for C++/ObjC++ but not for C"
print("\n[FIX] Separating and refining compiler flags for a clean build...")

def fix_flags():
    # Flags to separate
    cpp_only = [
        "-Wno-volatile",
        "-Wno-deprecated-enum-enum-conversion",
        "-Wno-extra-tokens"
    ]
    c_only = [
        "-Wno-discarded-qualifiers"
    ]

    # Common flags to suppress library noise and 3rd party issues
    # Added -Wno-stringop-overflow for helix-aac and other libraries
    common_suppression = [
        "-Wno-deprecated-declarations",
        "-Wno-cpp",
        "-Wno-stringop-overflow"
    ]

    # Helper to remove flag from an environment variable list or string
    def remove_flag(env_var, flag):
        flags = env.get(env_var, [])
        if isinstance(flags, list):
            while flag in flags:
                flags.remove(flag)
        elif isinstance(flags, str):
            if flag in flags:
                # Use regex or better string replacement to avoid partial matches
                import re
                env[env_var] = re.sub(r'\b' + re.escape(flag) + r'\b', '', flags).strip()

    # 1. Clean up shared CCFLAGS and ensure suppression is applied there as well
    for f in cpp_only + c_only + common_suppression:
        remove_flag("CCFLAGS", f)

    # Apply common suppression to CCFLAGS to ensure it's hit early and often
    for f in common_suppression:
        if f not in env.get("CCFLAGS", []):
            env.Append(CCFLAGS=[f])

    # 2. Add to language-specific flags
    # C++ Specific
    env.Append(CXXFLAGS=cpp_only)

    # C Specific
    env.Append(CFLAGS=c_only)

    # 3. Final safety cleanup of cross-pollination
    for f in cpp_only:
        remove_flag("CFLAGS", f)
    for f in c_only:
        remove_flag("CXXFLAGS", f)

# Execute the fix
fix_flags()
