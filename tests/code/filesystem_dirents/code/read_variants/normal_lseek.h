#ifndef NORMAL_LSEEK_H
#define NORMAL_LSEEK_H

#pragma once

#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::offset_spec_t> normal_lseek_variants = {
    {.offset = 0, .whence = 0},
    {.offset = -123, .whence = 0},
    {.offset = 123456, .whence = 0},
    {.offset = 60, .whence = 0},
    {.offset = 0, .whence = 1},
    {.offset = 24, .whence = 1},
    {.offset = -24, .whence = 1},
    {.offset = -6666, .whence = 1},
    {.offset = 123456, .whence = 1},
    {.offset = 0, .whence = 2},
    {.offset = 123456, .whence = 2},
    {.offset = 100, .whence = 2},
    {.offset = -100, .whence = 2},
    {.offset = -100000, .whence = 2},

    // cursed ones
    {.offset = 0, .whence = 3},
    {.offset = 8, .whence = 3},
    {.offset = 16, .whence = 3},
    {.offset = 24, .whence = 3},
    {.offset = 32, .whence = 3},
    {.offset = 40, .whence = 3},
    {.offset = 10240, .whence = 3},
    {.offset = -1024, .whence = 3},
    {.offset = -10240, .whence = 3},
    {.offset = -pfs_read_target - 1, .whence = 3},
    {.offset = -pfs_read_target, .whence = 3},
    {.offset = -pfs_read_target + 1, .whence = 3},

    {.offset = 0, .whence = 4},
    {.offset = 8, .whence = 4},
    {.offset = 16, .whence = 4},
    {.offset = 24, .whence = 4},
    {.offset = 32, .whence = 4},
    {.offset = 40, .whence = 4},
    {.offset = 10240, .whence = 4},
    {.offset = -1024, .whence = 4},
    {.offset = -10240, .whence = 4},
    {.offset = -pfs_read_target - 1, .whence = 4},
    {.offset = -pfs_read_target, .whence = 4},
    {.offset = -pfs_read_target + 1, .whence = 4},
};

#endif // NORMAL_LSEEK_H