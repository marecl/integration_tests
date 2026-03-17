#ifndef FS_TEST_CONST_H
#define FS_TEST_CONST_H

#include "fs_test.h"

#include <vector>

s64 oke264(int error) {
  return static_cast<s64>(error);
}

std::vector<OrbisInternals::DirentCombinationRead> normal_read_variants = {
    {0, 0, 0, 0},     {64, 0, 64, 0},   {128, 0, 128, 0}, //
    {256, 0, 256, 0}, {511, 0, 511, 0}, {511, 1, 511, 0}, //
    {511, 2, 511, 0}, {512, 0, 512, 0},
};
std::vector<OrbisInternals::DirentCombinationGetdirentries> normal_dirent_variants = {
    {.read_size = 0, .read_offset = 0, .expected_basep = 0, .expected_result = ORBIS_KERNEL_ERROR_EINVAL, .expected_errno = EINVAL},

    /**
    What i (again) think is that the entire 512byte buffer must be iterated? or sth
    read can occur as long as read descriptor passes 512aligned mark, i.e.
    it will read a couple of bytes if end position falls couple of bytes after the end
    for example read 8@510 offset will read 2 bytes
    2 bytes, because thet's the farthest the current sector can go
    however this is a bit different for larger reads. large reads still have this condition, but they include every buffer on their way

    so the only way this can read stuff is when end position crosses current sector border
    otherwise EINVAL
    */

    {0, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {128, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {256, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {511, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {513, 0, 0, 512, 0},
    {512, 0, 0, 512, 0},    //
    {512, -1, 512, 512, 0}, //
    {1024, 0, 0, 1024, 0},
    {1024, 511, 511, 513, 0},
    {1024, 512, 512, 1024, 0},
    {1024, 513, 513, 1023, 0},
    {1025, 513, 513, 1023, 0},
    {1026, 513, 513, 1023, 0},

    // try to get the filler between dirents
    {0, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {48, 64, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 64, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {48, 128, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 128, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 504, 504, 8, 0},
    {16, 496, 496, 16, 0},
    {24, 488, 488, 24, 0},
    {48, 464, 464, 48, 0},
    {64, 448, 448, 64, 0},
    {128, 384, 384, 128, 0},
    {256, 256, 256, 256, 0},
    {511, 1, 1, 511, 0},

    // {8, 4064, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    // {16, 4064, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    // {24, 4064, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    // {32, 4064, 4064, 32, 0},
    // {64, 4064, 4064, 32, 0},
    // {80, 4064, 4064, 32, 0},
    // {128, 4064, 4064, 32, 0},
    // {256, 4064, 4064, 32, 0},

    {1024, 0, 0, 1024, 0},
    {1024, -1, 1024, 1024, 0},
    {1024, -1, 2048, 1024, 0},

    {32, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {32, 480, 480, 32, 0},
    {32, 479, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
};

std::vector<OrbisInternals::DirentCombinationRead>          pfs_read_variants   = {};
std::vector<OrbisInternals::DirentCombinationGetdirentries> pfs_dirent_variants = {
    {.read_size = 0, .read_offset = 0, .expected_basep = 0, .expected_result = ORBIS_KERNEL_ERROR_EINVAL, .expected_errno = EINVAL},

    /**
    What i (again) think is that the entire 512byte buffer must be iterated? or sth
    read can occur as long as read descriptor passes 512aligned mark, i.e.
    it will read a couple of bytes if end position falls couple of bytes after the end
    for example read 8@510 offset will read 2 bytes
    2 bytes, because thet's the farthest the current sector can go
    however this is a bit different for larger reads. large reads still have this condition, but they include every buffer on their way
    for PFS any uneven read backs off to the earliest full dirent, except if it cannot backtrack (like <24 will skip [.] and jump to [..], i.e. going forward)

    so the only way this can read stuff is when end position crosses current sector border
    otherwise EINVAL

    apart from that i think it's a classic as before, end of read must appear to be after 512b alignment mark
    but reading from that
    */

    // bad reads - not reaching upper 512 byte mark
    {0, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {128, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {256, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {511, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 8, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 16, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 24, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {0, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {8, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {48, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 32, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {48, 64, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 64, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {48, 128, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {64, 128, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {0, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22}, // 512 is already treated as a new sector, so it won't backtrack
    {8, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {23, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {25, 512, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},

    {512, 0, 0, 496, 0},  //
    {512, -1, 496, 0, 0}, //
    {512, -1, 496, 0, 0}, //

    {512, 16, 16, 472, 0},
    {512, 512, 512, 480, 0}, // 496 - 975

    {513, 0, 0, 496, 0},
    {64, 534, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},  // seek to the middle of the dirent starting in the previous sector
    {64, 1015, 1015, 0, 0},                       // dirent would leak into new sector
    {64, 1016, 1016, 0, 0},                       // dirent would leak into new sector
    {64, 1017, 1017, 0, 0},                       // dirent would leak into new sector
    {64, 1024, 0, ORBIS_KERNEL_ERROR_EINVAL, 22}, // 1016 - 1055

    {1023, 0, 0, 496, 0},
    {1023, -1, 496, 520, 0},
    {1023, -1, 1016, 520, 0},

    {1024, 0, 0, 1016, 0},
    {1024, 511, 511, 480, 0},
    {1024, 512, 512, 1000, 0},
    {1024, 513, 513, 1000, 0},
    {1025, 513, 513, 1000, 0},
    {1026, 513, 513, 1000, 0},
    {1026, -1, 1513, 520, 0},

    {256, 256, 256, 240, 0},
    {256, -1, 496, 0, 0},

    // try to get the filler between dirents

    {23, 511, 511, 0, 0},
    {24, 511, 511, 0, 0},
    {25, 511, 511, 0, 0},

    {8, 504, 504, 0, 0},
    {16, 496, 496, 0, 0},
    {24, 488, 488, 0, 0},
    {48, 464, 464, 40, 0},
    {64, 448, 448, 40, 0},
    {128, 384, 384, 120, 0},
    {256, 256, 256, 240, 0},
    {511, 1, 1, 472, 0},

    {8, 4064, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {16, 4064, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {24, 4064, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {32, 4064, 4064, 0, 0}, // not a full dirent but valid
    {64, 4064, 4064, 0, 0}, // doesnt pass alignment
    {80, 4064, 4064, 0, 0},
    {112, 4064, 4064, 0, 0}, // passes alignment brom the bottom but doesn't have a full dirent and doesn't pass upper boundary
    {128, 4064, 4064, 0, 0}, //
    {256, 4064, 4064, 0, 0},
    {512 + 32, 4064, 4064, 504, 0},

    {1024, 0, 0, 1016, 0},
    {1024, -1, 1016, 520, 0},
    {1024, -1, 1536, 1016, 0},

    {32, 0, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
    {32, 480, 480, 0, 0},
    {32, 479, 0, ORBIS_KERNEL_ERROR_EINVAL, 22},
};

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