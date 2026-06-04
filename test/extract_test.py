import sys

def extract_function(filepath, func_signature):
    with open(filepath, 'r') as f:
        content = f.read()

    start_idx = content.find(func_signature)
    if start_idx == -1:
        print("Function signature not found")
        sys.exit(1)

    start_brace = content.find('{', start_idx)
    if start_brace == -1:
        sys.exit(1)

    brackets = 0
    end_idx = -1
    for i in range(start_brace, len(content)):
        if content[i] == '{':
            brackets += 1
        elif content[i] == '}':
            brackets -= 1
            if brackets == 0:
                end_idx = i
                break

    if end_idx != -1:
        func_body = content[start_idx:end_idx+1]
        with open('setup_gpio_impl.cpp', 'w') as out:
            out.write(func_body)
            out.write("\n")
    else:
        print("Could not find matching brackets")
        sys.exit(1)

if __name__ == "__main__":
    extract_function("../src/main.cpp", "void setup_gpio() {")
