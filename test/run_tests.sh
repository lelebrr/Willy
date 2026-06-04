#!/bin/bash
set -e

# Change directory to the root of the project
cd "$(dirname "$0")/.."

echo "Running storage tests..."
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp
g++ -I./test test/test_sd_functions.cpp -o test/test_sd_functions_runner
./test/test_sd_functions_runner
rm -f test/sd_functions_extracted.cpp test/test_sd_functions_runner

# Extract getHierarchicalPath from src/core/sd_functions.cpp
sed -n '/^\/\/ --- BEGIN_GET_HIERARCHICAL_PATH_TEST_EXTRACT ---$/,/^\/\/ --- END_GET_HIERARCHICAL_PATH_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_GET_HIERARCHICAL_PATH_TEST_EXTRACT" | grep -v "END_GET_HIERARCHICAL_PATH_TEST_EXTRACT" > test/sd_functions_get_hierarchical_path_extracted.cpp

# Compile tests
g++ -I./test test/test_sd_functions.cpp -o test/test_runner

# Run tests
./test/test_runner
rm -f test/test_runner test/setup_gpio_impl.cpp

echo "Running passwords tests..."
# Extract passwords test code
sed -n '/^\/\/ --- BEGIN_XOR_TEST_EXTRACT ---$/,/^\/\/ --- END_XOR_TEST_EXTRACT ---$/p' src/core/passwords.cpp | grep -v "BEGIN_XOR_TEST_EXTRACT" | grep -v "END_XOR_TEST_EXTRACT" > test/passwords_extracted.cpp
sed -n '/^\/\/ --- BEGIN_PASSWORDS_TEST_EXTRACT ---$/,/^\/\/ --- END_PASSWORDS_TEST_EXTRACT ---$/p' src/core/passwords.cpp | grep -v "BEGIN_PASSWORDS_TEST_EXTRACT" | grep -v "END_PASSWORDS_TEST_EXTRACT" >> test/passwords_extracted.cpp

# Clean up
rm -f test/sd_functions_extracted.cpp
rm -f test/sd_functions_get_hierarchical_path_extracted.cpp
rm -f test/test_runner

# Exit with test runner result
exit $RESULT
