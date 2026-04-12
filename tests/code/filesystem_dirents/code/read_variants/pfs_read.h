#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::DirentCombinationRead> pfs_read_variants = {
    {.read_size = 0, .read_offset = 0, .expected_result = 0, .expected_end_position = 0},                //  0
    {.read_size = 64, .read_offset = 0, .expected_result = 64, .expected_end_position = 64},             //  64
    {.read_size = 128, .read_offset = 0, .expected_result = 128, .expected_end_position = 128},          //  128
    {.read_size = 256, .read_offset = 0, .expected_result = 256, .expected_end_position = 256},          //  256
    {.read_size = 511, .read_offset = 0, .expected_result = 511, .expected_end_position = 511},          //  511
    {.read_size = 511, .read_offset = 1, .expected_result = 511, .expected_end_position = 512},          //  512
    {.read_size = 511, .read_offset = 2, .expected_result = 511, .expected_end_position = 513},          //  513
    {.read_size = 512, .read_offset = 0, .expected_result = 512, .expected_end_position = 512},          //  512
    {.read_size = 2048, .read_offset = 1245, .expected_result = 2048, .expected_end_position = 3293},    //  3293
    {.read_size = 7257, .read_offset = 1245, .expected_result = 7257, .expected_end_position = 8502},    //  8502
    {.read_size = 418, .read_offset = 574, .expected_result = 418, .expected_end_position = 992},        //  992
    {.read_size = 9363, .read_offset = 1111, .expected_result = 9363, .expected_end_position = 10474},   //  10474
    {.read_size = 37865, .read_offset = 936, .expected_result = 37865, .expected_end_position = 38801},  //  38801
    {.read_size = 17543, .read_offset = 1245, .expected_result = 17543, .expected_end_position = 18788}, //  18788
    {.read_size = 1024, .read_offset = 35565, .expected_result = 1024, .expected_end_position = 36589},  //  36589
    {.read_size = 512, .read_offset = 65534, .expected_result = 2, .expected_end_position = 65536},      //  65536
    {.read_size = 2048, .read_offset = 65534, .expected_result = 2, .expected_end_position = 65536},     //  65536
    {.read_size = 4096, .read_offset = 8192, .expected_result = 4096, .expected_end_position = 12288},   //  12288
    {.read_size = 1024, .read_offset = 8000, .expected_result = 1024, .expected_end_position = 9024},    //  9024
    {.read_size = 1024, .read_offset = 10000, .expected_result = 1024, .expected_end_position = 11024},  //  11024
    {.read_size = 4096, .read_offset = 10000, .expected_result = 4096, .expected_end_position = 14096},  //  14096
};