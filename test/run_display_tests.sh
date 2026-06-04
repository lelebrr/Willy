#!/bin/bash

# Extract the padprint functions from src/core/display.cpp using the markers
sed -n '/^\/\/ --- BEGIN_PADPRINT_TEST_EXTRACT ---$/,/^\/\/ --- END_PADPRINT_TEST_EXTRACT ---$/p' src/core/display.cpp | grep -v "BEGIN_PADPRINT_TEST_EXTRACT" | grep -v "END_PADPRINT_TEST_EXTRACT" > test/display_padprint_extracted.cpp

# Compile tests
g++ -I./test test/test_display_padprint.cpp -o test/test_display_runner

# Run tests
./test/test_display_runner

# Clean up
rm -f test/display_padprint_extracted.cpp
rm -f test/test_display_runner
