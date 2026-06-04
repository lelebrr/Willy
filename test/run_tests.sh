#!/bin/bash
set -e

# Change directory to the root of the project
cd "$(dirname "$0")/.."

echo "Running storage tests..."
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp
g++ -I./test test/test_sd_functions.cpp -o test/test_sd_functions_runner
./test/test_sd_functions_runner
rm -f test/sd_functions_extracted.cpp test/test_sd_functions_runner

echo "Running gpio tests..."
cd test
python3 extract_test.py
cd ..
g++ -I./test test/test_main_setup_gpio.cpp -o test/test_runner
./test/test_runner
rm -f test/test_runner test/setup_gpio_impl.cpp

echo "Running passwords tests..."
# Extract passwords test code
sed -n '/^\/\/ --- BEGIN_XOR_TEST_EXTRACT ---$/,/^\/\/ --- END_XOR_TEST_EXTRACT ---$/p' src/core/passwords.cpp | grep -v "BEGIN_XOR_TEST_EXTRACT" | grep -v "END_XOR_TEST_EXTRACT" > test/passwords_extracted.cpp
sed -n '/^\/\/ --- BEGIN_PASSWORDS_TEST_EXTRACT ---$/,/^\/\/ --- END_PASSWORDS_TEST_EXTRACT ---$/p' src/core/passwords.cpp | grep -v "BEGIN_PASSWORDS_TEST_EXTRACT" | grep -v "END_PASSWORDS_TEST_EXTRACT" >> test/passwords_extracted.cpp

g++ -I./test test/test_passwords.cpp -o test/test_passwords_runner
./test/test_passwords_runner
rm -f test/passwords_extracted.cpp test/test_passwords_runner
