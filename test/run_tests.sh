#!/bin/bash
set -e

# Keep track of overall failure
FAILED=0

# Extract begin_storage function from src/main.cpp using the markers
sed -n '/^\/\/ --- BEGIN_STORAGE_TEST_EXTRACT ---$/,/^\/\/ --- END_STORAGE_TEST_EXTRACT ---$/p' src/main.cpp | grep -v "BEGIN_STORAGE_TEST_EXTRACT" | grep -v "END_STORAGE_TEST_EXTRACT" > test/sd_functions_extracted.cpp
g++ -I./test test/test_sd_functions.cpp -o test/test_sd_functions_runner
./test/test_sd_functions_runner
rm -f test/test_sd_functions_runner

# Extract getHierarchicalPath from src/core/sd_functions.cpp
sed -n '/^\/\/ --- BEGIN_GET_HIERARCHICAL_PATH_TEST_EXTRACT ---$/,/^\/\/ --- END_GET_HIERARCHICAL_PATH_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_GET_HIERARCHICAL_PATH_TEST_EXTRACT" | grep -v "END_GET_HIERARCHICAL_PATH_TEST_EXTRACT" > test/sd_functions_get_hierarchical_path_extracted.cpp

# Extract setupSdCard from src/core/sd_functions.cpp using the markers
sed -n '/^\/\/ --- BEGIN_SD_SETUP_TEST_EXTRACT ---$/,/^\/\/ --- END_SD_SETUP_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_SD_SETUP_TEST_EXTRACT" | grep -v "END_SD_SETUP_TEST_EXTRACT" > test/sd_setup_extracted.cpp

# Extract checkLittleFsSize, checkLittleFsSizeNM, getFsStorage from src/core/sd_functions.cpp using the markers
sed -n '/^\/\/ --- BEGIN_SD_FS_TEST_EXTRACT ---$/,/^\/\/ --- END_SD_FS_TEST_EXTRACT ---$/p' src/core/sd_functions.cpp | grep -v "BEGIN_SD_FS_TEST_EXTRACT" | grep -v "END_SD_FS_TEST_EXTRACT" > test/sd_fs_extracted.cpp

# Compile sd_functions tests
g++ -I./test test/test_sd_functions.cpp -o test/test_runner_sd

# Run sd_functions tests
./test/test_runner_sd || FAILED=1

# Compile file_utils tests
g++ -I./test test/test_file_utils.cpp src/core/file_utils.cpp -o test/test_runner_file_utils

# Run file_utils tests
./test/test_runner_file_utils || FAILED=1

# Compile net_utils tests
g++ -I./test test/test_net_utils.cpp src/core/net_utils.cpp -o test/test_runner_net_utils

# Run net_utils tests
./test/test_runner_net_utils || FAILED=1

# Clean up
rm -f test/sd_functions_extracted.cpp
rm -f test/test_runner_sd
rm -f test/test_runner_file_utils
rm -f test/test_runner_net_utils

# Exit with test runner result
exit $FAILED
