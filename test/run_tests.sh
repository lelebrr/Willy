#!/bin/bash

# Keep track of overall failure
FAILED=0

# Extract begin_storage function from src/main.cpp using the markers
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp

# Compile sd_functions tests
g++ -I./test test/test_sd_functions.cpp -o test/test_runner_sd

# Run sd_functions tests
./test/test_runner_sd || FAILED=1

# Compile file_utils tests
g++ -I./test test/test_file_utils.cpp src/core/file_utils.cpp -o test/test_runner_file_utils

# Run file_utils tests
./test/test_runner_file_utils || FAILED=1

# Clean up
rm -f test/sd_functions_extracted.cpp
rm -f test/test_runner_sd
rm -f test/test_runner_file_utils

# Exit with test runner result
exit $FAILED
