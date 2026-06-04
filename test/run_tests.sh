#!/bin/bash

# Extract begin_storage function from src/main.cpp using the markers
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp

# Extract padprint functions from src/core/display.cpp using the markers
sed -n '/^\/\/ --- BEGIN_PADPRINT_TEST_EXTRACT ---$/,/^\/\/ --- END_PADPRINT_TEST_EXTRACT ---$/p' src/core/display.cpp | grep -v "BEGIN_PADPRINT_TEST_EXTRACT" | grep -v "END_PADPRINT_TEST_EXTRACT" > test/display_padprint_extracted.cpp

# Compile tests
g++ -I./test test/test_sd_functions.cpp -o test/test_runner_sd
g++ -I./test test/test_display_padprint.cpp -o test/test_runner_display

# Run tests
./test/test_runner_sd
RESULT_SD=$?

./test/test_runner_display
RESULT_DISPLAY=$?

# Combine results
if [ $RESULT_SD -ne 0 ] || [ $RESULT_DISPLAY -ne 0 ]; then
    RESULT=1
else
    RESULT=0
fi

# Clean up
rm -f test/sd_functions_extracted.cpp
rm -f test/test_runner_sd
rm -f test/display_padprint_extracted.cpp
rm -f test/test_runner_display

# Exit with test runner result
exit $RESULT
