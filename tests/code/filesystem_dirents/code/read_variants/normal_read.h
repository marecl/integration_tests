#include "../fs_test.h"

#include <vector>

// read size, read offset // HW return, HW end position
std::vector<std::pair<s64, s64>> normal_read_variants = {
    {0, 0},        // return = 0    ,   endpos = 0
    {64, 0},       // return = 64   ,   endpos = 64
    {128, 0},      // return = 128  ,   endpos = 128
    {256, 0},      // return = 256  ,   endpos = 256
    {511, 0},      // return = 511  ,   endpos = 511
    {511, 1},      // return = 511  ,   endpos = 512
    {511, 2},      // return = 511  ,   endpos = 513
    {512, 0},      // return = 512  ,   endpos = 512
    {2048, 1245},  // return = 2048 ,   endpos = 3293
    {7257, 1245},  // return = 7257 ,   endpos = 8502
    {418, 574},    // return = 418  ,   endpos = 992
    {9363, 1111},  // return = 7593 ,   endpos = 8704
    {37865, 936},  // return = 7768 ,   endpos = 8704
    {17543, 1245}, // return = 7459 ,   endpos = 8704
    {1024, 35565}, // return = 0    ,   endpos = 35565
    {512, 65534},  // return = 0    ,   endpos = 65534
    {2048, 65534}, // return = 0    ,   endpos = 65534
    {4096, 8192},  // return = 512  ,   endpos = 8704
    {1024, 8000},  // return = 704  ,   endpos = 8704
    {1024, 10000}, // return = 0    ,   endpos = 10000
    {4096, 10000}, // return = 0    ,   endpos = 10000
};
