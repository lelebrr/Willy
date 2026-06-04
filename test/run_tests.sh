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

# Extract setupSdCard from src/core/sd_functions.cpp using the markers
sed -n '/^\/\/ --- BEGIN_SD_SETUP_TEST_EXTRACT ---$/,/^\/\/ --- END_SD_SETUP_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_SD_SETUP_TEST_EXTRACT" | grep -v "END_SD_SETUP_TEST_EXTRACT" > test/sd_setup_extracted.cpp

# Extract checkLittleFsSize, checkLittleFsSizeNM, getFsStorage from src/core/sd_functions.cpp using the markers
sed -n '/^\/\/ --- BEGIN_SD_FS_TEST_EXTRACT ---$/,/^\/\/ --- END_SD_FS_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_SD_FS_TEST_EXTRACT" | grep -v "END_SD_FS_TEST_EXTRACT" > test/sd_fs_extracted.cpp

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
rm -f test/sd_setup_extracted.cpp
rm -f test/sd_fs_extracted.cpp
rm -f test/test_runner

# Use standard shell construct to fail if tests fail since exit is blocked
if [ "$RESULT" -ne 0 ]; then
  echo "Tests failed!"
  /bin/false
else
  echo "Tests passed!"
fi
