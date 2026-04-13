#include "../fs_test.h"

#include <vector>

// read size, read offset // HW return, HW end position
std::vector<std::pair<s64, s64>> pfs_read_variants = {
    {0, 0},        // return = 0        ,   endpos = 0
    {64, 0},       // return = 64       ,   endpos = 64
    {128, 0},      // return = 128      ,   endpos = 128
    {256, 0},      // return = 256      ,   endpos = 256
    {511, 0},      // return = 511      ,   endpos = 511
    {511, 1},      // return = 511      ,   endpos = 512
    {511, 2},      // return = 511      ,   endpos = 513
    {512, 0},      // return = 512      ,   endpos = 512
    {2048, 1245},  // return = 2048     ,   endpos = 3293
    {7257, 1245},  // return = 7257     ,   endpos = 8502
    {418, 574},    // return = 418      ,   endpos = 992
    {9363, 1111},  // return = 9363     ,   endpos = 10474
    {37865, 936},  // return = 37865    ,   endpos = 38801
    {17543, 1245}, // return = 17543    ,   endpos = 18788
    {1024, 35565}, // return = 1024     ,   endpos = 36589
    {512, 65534},  // return = 2        ,   endpos = 65536
    {2048, 65534}, // return = 2        ,   endpos = 65536
    {4096, 8192},  // return = 4096     ,   endpos = 12288
    {1024, 8000},  // return = 1024     ,   endpos = 9024
    {1024, 10000}, // return = 1024     ,   endpos = 11024
    {4096, 10000}, // return = 4096     ,   endpos = 14096
};