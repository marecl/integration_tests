#include "../fs_test.h"

#include <vector>

// read size, read offset // HW basep, HW return, HW end position
std::vector<std::pair<s64, s64>> pfs_dirent_variants = {

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

    {512, 0},      // 0        496        496
    {512, -1},     // 496      0          496
    {512, -1},     // 496      0          496
    {512, 16},     // 16       472        488
    {512, 512},    // 512      480        992
    {513, 0},      // 0        496        496
    {536, 0},      // 0        496        496
    {64, 1015},    // 1015     0          1015
    {64, 1016},    // 1016     0          1016
    {64, 1017},    // 1017     0          1017
    {80, 1015},    // 1015     0          1015
    {80, 1016},    // 1016     0          1016
    {80, 1017},    // 1017     0          1017
    {1023, 0},     // 0        496        496
    {1023, -1},    // 496      520        1016
    {1023, -1},    // 1016     520        1536
    {1024, 0},     // 0        1016       1016
    {1024, 511},   // 511      480        991
    {1024, 512},   // 512      1000       1512
    {1024, 513},   // 513      1000       1513
    {1025, 513},   // 513      1000       1513
    {1026, 513},   // 513      1000       1513
    {1026, -1},    // 1513     520        2033
    {256, 256},    // 256      240        496
    {256, -1},     // 496      0          496
    {23, 511},     // 511      0          511
    {24, 511},     // 511      0          511
    {25, 511},     // 511      0          511
    {8, 504},      // 504      0          504
    {16, 496},     // 496      0          496
    {24, 488},     // 488      0          488
    {48, 464},     // 464      40         504
    {64, 448},     // 448      40         488
    {128, 384},    // 384      120        504
    {256, 256},    // 256      240        496
    {511, 1},      // 1        472        473
    {32, 4064},    // 4064     0          4064
    {64, 4064},    // 4064     0          4064
    {80, 4064},    // 4064     0          4064
    {112, 4064},   // 4064     0          4064
    {128, 4064},   // 4064     0          4064
    {256, 4064},   // 4064     0          4064
    {544, 4064},   // 4064     504        4568
    {1024, 0},     // 0        1016       1016
    {1024, -1},    // 1016     520        1536
    {1024, -1},    // 1536     1000       2536
    {32, 480},     // 480      0          480
    {1024, 10000}, // 10000    648        65536
    {8192, 10000}, // 10000    648        65536
    {8192, 35565}, // 35565    0          65536
};