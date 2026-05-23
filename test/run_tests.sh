#!/bin/bash

# Extract begin_storage function from src/main.cpp using the markers
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp

# Compile tests
g++ -I./test test/test_sd_functions.cpp -o test/test_runner

# Run tests
./test/test_runner

# Check result
RESULT=$?

# Clean up
rm -f test/sd_functions_extracted.cpp
rm -f test/test_runner

# Exit with test runner result
exit $RESULT
