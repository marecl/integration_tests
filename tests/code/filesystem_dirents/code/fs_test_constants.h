#ifndef FS_TEST_CONST_H
#define FS_TEST_CONST_H

#include "fs_test.h"

#include <vector>

std::vector<OrbisInternals::DirentCombination> normal_read_variants = {
    {0, 0, 0, 0}, {64, 0, 64, 0}, {128, 0, 128, 0}, {256, 0, 256, 0}, {511, 0, 511, 0}, {511, 1, 511, 0}, {511, 2, 511, 0}, {512, 0, 512, 0},
};
std::vector<OrbisInternals::DirentCombination> normal_dirent_variants = {};
std::vector<OrbisInternals::DirentCombination> pfs_read_variants      = {};
std::vector<OrbisInternals::DirentCombination> pfs_dirent_variants    = {};

const unsigned char pfs_dirent_entry_dot[24] = {
    0x00, 0x00, 0x00, 0x00,                   // 4 fileno
    0x18, 0x00, 0x04, 0x01,                   // 2 reclen 1 type 1 namelen
    0x2E, 0x00,                               // name
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // padding
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // padding
};

const unsigned char pfs_dirent_entry_dotdot[24] = {
    0x00, 0x00, 0x00, 0x00,                   // 4 fileno
    0x18, 0x00, 0x04, 0x02,                   // 2 reclen 1 type 1 namelen
    0x2E, 0x2E, 0x00,                         // name
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // padding to 8
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00        // padding to 8
};

const unsigned char pfs_read_entry_dot[24] = {
    0x00, 0x00, 0x00, 0x00,            // 4 fileno
    0x18, 0x00, 0x00, 0x00,            // reclen
    0x04, 0x00, 0x00, 0x00,            // type
    0x01, 0x00, 0x00, 0x00,            // namelen
    0x2E, 0x00,                        // name
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding to 8
};

const unsigned char pfs_read_entry_dotdot[24] = {
    0x00, 0x00, 0x00, 0x00,      // 4 fileno
    0x18, 0x00, 0x00, 0x00,      // reclen
    0x04, 0x00, 0x00, 0x00,      // type
    0x02, 0x00, 0x00, 0x00,      // namelen
    0x2E, 0x2E, 0x00,            // name
    0x00, 0x00, 0x00, 0x00, 0x00 // padding to 8
};

#endif