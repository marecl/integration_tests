#include "../fs_test.h"

#include <vector>

// expected result is here only for visualization, real good values are calculated for each type individually
std::vector<OrbisInternals::DirentCombinationRead> normal_read_variants = {
    {.read_size = 0, .read_offset = 0, .expected_result = 0, .expected_end_position = 0},              // 0
    {.read_size = 64, .read_offset = 0, .expected_result = 64, .expected_end_position = 64},           // 64
    {.read_size = 128, .read_offset = 0, .expected_result = 128, .expected_end_position = 128},        // 128
    {.read_size = 256, .read_offset = 0, .expected_result = 256, .expected_end_position = 256},        // 256
    {.read_size = 511, .read_offset = 0, .expected_result = 511, .expected_end_position = 511},        // 511
    {.read_size = 511, .read_offset = 1, .expected_result = 511, .expected_end_position = 512},        // 512
    {.read_size = 511, .read_offset = 2, .expected_result = 511, .expected_end_position = 513},        // 513
    {.read_size = 512, .read_offset = 0, .expected_result = 512, .expected_end_position = 512},        // 512
    {.read_size = 2048, .read_offset = 1245, .expected_result = 2048, .expected_end_position = 3293},  // 3293
    {.read_size = 7257, .read_offset = 1245, .expected_result = 7257, .expected_end_position = 8502},  // 8502
    {.read_size = 418, .read_offset = 574, .expected_result = 418, .expected_end_position = 992},      // 992
    {.read_size = 9363, .read_offset = 1111, .expected_result = 7593, .expected_end_position = 8704},  // 8704
    {.read_size = 37865, .read_offset = 936, .expected_result = 7768, .expected_end_position = 8704},  // 8704
    {.read_size = 17543, .read_offset = 1245, .expected_result = 7459, .expected_end_position = 8704}, // 8704
    {.read_size = 1024, .read_offset = 35565, .expected_result = 0, .expected_end_position = 35565},   // 35565
    {.read_size = 512, .read_offset = 65534, .expected_result = 0, .expected_end_position = 65534},    // 65534
    {.read_size = 2048, .read_offset = 65534, .expected_result = 0, .expected_end_position = 65534},   // 65534
    {.read_size = 4096, .read_offset = 8192, .expected_result = 512, .expected_end_position = 8704},   // 8704
    {.read_size = 1024, .read_offset = 8000, .expected_result = 704, .expected_end_position = 8704},   // 8704
    {.read_size = 1024, .read_offset = 10000, .expected_result = 0, .expected_end_position = 10000},   // 10000
    {.read_size = 4096, .read_offset = 10000, .expected_result = 0, .expected_end_position = 10000},   // 10000
};
