#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::DirentCombinationGetdirentries> pfs_dirent_variants = {

    /**
     * Final writeup:
     *  1.  offset + read size MUST break any 512b alignment before attempting to read
     *      offset+size=read 0+(<512), 256+(<256) BUT 511+16=0
     */

    /**
    What i (again) think is that the entire 512byte buffer must be iterated? or sth
    read can occur as long as read descriptor passes 512aligned mark, i.e.
    it will read a couple of bytes if end position falls couple of bytes after the end
    for example read 8@510 offset will read 2 bytes
    2 bytes, because thet's the farthest the current sector can go
    however this is a bit different for larger reads. large reads still have this condition, but they include every buffer on their way
    for PFS any uneven read backs off to the earliest full dirent, except if it cannot backtrack (like <24 will skip [.] and jump to [..], i.e. going
    forward)

    so the only way this can read stuff is when end position crosses current sector border
    otherwise EINVAL

    apart from that i think it's a classic as before, end of read must appear to be after 512b alignment mark
    but reading from that
    */

    /**
     * Apparent end crosses sector end - sector is read
     * Apparent end doesn't cross sector end - everything from that sector is omitted
     *
     * Read start
     */

    // apparent end = offset + length before checking dirents

    // 512 byte reads can get deadlocked like here. 496 read, so next 512 read won't break 1024 barrier
    {.read_size = 512, .read_offset = 0, .expected_basep = 0, .expected_result = 496, .expected_end_position = 496}, // 0 - 496, reaches upper border
    // 496 + 512 = 1008 < 1024, apparent end below sector end
    {.read_size = 512, .read_offset = -1, .expected_basep = 496, .expected_result = 0, .expected_end_position = 496},
    {.read_size = 512, .read_offset = -1, .expected_basep = 496, .expected_result = 0, .expected_end_position = 496},  // same as above
    {.read_size = 512, .read_offset = 16, .expected_basep = 16, .expected_result = 472, .expected_end_position = 488}, // 16 - 496, reaches upper border
    {.read_size             = 512,
     .read_offset           = 512,
     .expected_basep        = 512,
     .expected_result       = 480,
     .expected_end_position = 992},                                                                                  // 496 - 975, apparent end at sector end
    {.read_size = 513, .read_offset = 0, .expected_basep = 0, .expected_result = 496, .expected_end_position = 496}, // 0 - 496, apparent end beyond sector end
    // 0 - 496, end on dirent in next sector, does not read from next sector, apparent end doesnt reach its end border
    {.read_size = 536, .read_offset = 0, .expected_basep = 0, .expected_result = 496, .expected_end_position = 496},
    {.read_size = 64, .read_offset = 1015, .expected_basep = 1015, .expected_result = 0, .expected_end_position = 1015}, // dirent would leak into new sector
    {.read_size = 64, .read_offset = 1016, .expected_basep = 1016, .expected_result = 0, .expected_end_position = 1016}, // dirent would leak into new sector
    {.read_size = 64, .read_offset = 1017, .expected_basep = 1017, .expected_result = 0, .expected_end_position = 1017}, // dirent would leak into new sector
    {.read_size = 80, .read_offset = 1015, .expected_basep = 1015, .expected_result = 0, .expected_end_position = 1015}, // dirent would leak into new sector
    {.read_size = 80, .read_offset = 1016, .expected_basep = 1016, .expected_result = 0, .expected_end_position = 1016}, // dirent would leak into new sector
    {.read_size = 80, .read_offset = 1017, .expected_basep = 1017, .expected_result = 0, .expected_end_position = 1017}, // dirent would leak into new sector
    {.read_size = 1023, .read_offset = 0, .expected_basep = 0, .expected_result = 496, .expected_end_position = 496},
    {.read_size = 1023, .read_offset = -1, .expected_basep = 496, .expected_result = 520, .expected_end_position = 1016},
    {.read_size = 1023, .read_offset = -1, .expected_basep = 1016, .expected_result = 520, .expected_end_position = 1536},
    {.read_size = 1024, .read_offset = 0, .expected_basep = 0, .expected_result = 1016, .expected_end_position = 1016},
    {.read_size = 1024, .read_offset = 511, .expected_basep = 511, .expected_result = 480, .expected_end_position = 991},
    {.read_size = 1024, .read_offset = 512, .expected_basep = 512, .expected_result = 1000, .expected_end_position = 1512},
    {.read_size = 1024, .read_offset = 513, .expected_basep = 513, .expected_result = 1000, .expected_end_position = 1513},
    {.read_size = 1025, .read_offset = 513, .expected_basep = 513, .expected_result = 1000, .expected_end_position = 1513},
    {.read_size = 1026, .read_offset = 513, .expected_basep = 513, .expected_result = 1000, .expected_end_position = 1513},
    {.read_size = 1026, .read_offset = -1, .expected_basep = 1513, .expected_result = 520, .expected_end_position = 2033},

    {.read_size = 256, .read_offset = 256, .expected_basep = 256, .expected_result = 240, .expected_end_position = 496},
    {.read_size = 256, .read_offset = -1, .expected_basep = 496, .expected_result = 0, .expected_end_position = 496},

    // try to get the filler between dirents

    {.read_size = 23, .read_offset = 511, .expected_basep = 511, .expected_result = 0, .expected_end_position = 511},
    {.read_size = 24, .read_offset = 511, .expected_basep = 511, .expected_result = 0, .expected_end_position = 511},
    {.read_size = 25, .read_offset = 511, .expected_basep = 511, .expected_result = 0, .expected_end_position = 511},
    {.read_size = 8, .read_offset = 504, .expected_basep = 504, .expected_result = 0, .expected_end_position = 504},
    {.read_size = 16, .read_offset = 496, .expected_basep = 496, .expected_result = 0, .expected_end_position = 496},
    {.read_size = 24, .read_offset = 488, .expected_basep = 488, .expected_result = 0, .expected_end_position = 488},
    {.read_size = 48, .read_offset = 464, .expected_basep = 464, .expected_result = 40, .expected_end_position = 504},
    {.read_size = 64, .read_offset = 448, .expected_basep = 448, .expected_result = 40, .expected_end_position = 488},
    {.read_size = 128, .read_offset = 384, .expected_basep = 384, .expected_result = 120, .expected_end_position = 504},
    {.read_size = 256, .read_offset = 256, .expected_basep = 256, .expected_result = 240, .expected_end_position = 496},
    {.read_size = 511, .read_offset = 1, .expected_basep = 1, .expected_result = 472, .expected_end_position = 473},
    {.read_size = 32, .read_offset = 4064, .expected_basep = 4064, .expected_result = 0, .expected_end_position = 4064}, // not a full dirent but valid
    {.read_size = 64, .read_offset = 4064, .expected_basep = 4064, .expected_result = 0, .expected_end_position = 4064}, // doesnt pass alignment
    {.read_size = 80, .read_offset = 4064, .expected_basep = 4064, .expected_result = 0, .expected_end_position = 4064},
    // passes alignment brom the bottom but doesn't have a full dirent and doesn't pass upper boundary
    {.read_size = 112, .read_offset = 4064, .expected_basep = 4064, .expected_result = 0, .expected_end_position = 4064},
    {.read_size = 128, .read_offset = 4064, .expected_basep = 4064, .expected_result = 0, .expected_end_position = 4064}, //
    {.read_size = 256, .read_offset = 4064, .expected_basep = 4064, .expected_result = 0, .expected_end_position = 4064},
    {.read_size = 544, .read_offset = 4064, .expected_basep = 4064, .expected_result = 504, .expected_end_position = 4568},
    {.read_size = 1024, .read_offset = 0, .expected_basep = 0, .expected_result = 1016, .expected_end_position = 1016},
    {.read_size = 1024, .read_offset = -1, .expected_basep = 1016, .expected_result = 520, .expected_end_position = 1536},
    {.read_size = 1024, .read_offset = -1, .expected_basep = 1536, .expected_result = 1016, .expected_end_position = 2552},
    {.read_size = 32, .read_offset = 480, .expected_basep = 480, .expected_result = 0, .expected_end_position = 480},
    {.read_size = 1024, .read_offset = 10000, .expected_basep = 10000, .expected_result = 576, .expected_end_position = 65536},
    {.read_size = 8192, .read_offset = 10000, .expected_basep = 10000, .expected_result = 576, .expected_end_position = 65536},
    {.read_size = 8192, .read_offset = 35565, .expected_basep = 35565, .expected_result = 0, .expected_end_position = 65536},
};