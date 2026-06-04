#!/bin/bash

# Extract begin_storage function from src/main.cpp using the markers
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp

# Extract setupSdCard from src/core/sd_functions.cpp using the markers
sed -n '/^\/\/ --- BEGIN_SD_SETUP_TEST_EXTRACT ---$/,/^\/\/ --- END_SD_SETUP_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_SD_SETUP_TEST_EXTRACT" | grep -v "END_SD_SETUP_TEST_EXTRACT" > test/sd_setup_extracted.cpp

# Extract checkLittleFsSize, checkLittleFsSizeNM, getFsStorage from src/core/sd_functions.cpp using the markers
sed -n '/^\/\/ --- BEGIN_SD_FS_TEST_EXTRACT ---$/,/^\/\/ --- END_SD_FS_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_SD_FS_TEST_EXTRACT" | grep -v "END_SD_FS_TEST_EXTRACT" > test/sd_fs_extracted.cpp

# Compile tests
g++ -I./test test/test_sd_functions.cpp -o test/test_runner

# Run tests
./test/test_runner

# Check result
RESULT=$?

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
