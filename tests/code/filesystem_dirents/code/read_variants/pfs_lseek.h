#ifndef PFS_LSEEK_H
#define PFS_LSEEK_H

#pragma once

#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::offset_spec_t> pfs_lseek_variants = {
    {.offset = 0, .whence = 0},      {.offset = -123, .whence = 0},    {.offset = 123456, .whence = 0}, {.offset = 60, .whence = 0},
    {.offset = 0, .whence = 1},      {.offset = 24, .whence = 1},      {.offset = -24, .whence = 1},    {.offset = -6666, .whence = 1},
    {.offset = 123456, .whence = 1}, {.offset = 0, .whence = 2},       {.offset = 123456, .whence = 2}, {.offset = 100, .whence = 2},
    {.offset = -100, .whence = 2},   {.offset = -100000, .whence = 2},
};

#endif // PFS_LSEEK_H
