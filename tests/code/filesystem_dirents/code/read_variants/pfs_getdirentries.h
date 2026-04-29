#include "../fs_test.h"

#include <vector>

// read size, read offset // HW basep, HW return, HW end position
std::vector<OrbisInternals::spec_t> pfs_dirent_variants = {

    /**
     * Final writeup:
     * 1. offset + read size MUST break any 512b alignment before attempting to read
     * offset+size=read 0+(<512), 256+(<256) BUT 511+16=0
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

    {.size = 0, .offset = 0},        // 170     EINVAL  0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 128, .offset = 128},    // 170     EINVAL  128     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 1024},   // 1024    480     1504    280000002800081066696c656e616d65
    {.size = 128, .offset = 128},    // 170     EINVAL  128     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 0},      // 0       496     496     07000000180004012e00000000000000
    {.size = 512, .offset = 16},     // 16      472     488     03000000180004022e2e000000000000
    {.size = 512, .offset = 24},     // 24      472     496     03000000180004022e2e000000000000
    {.size = 512, .offset = 28},     // 28      448     476     0c000000180008016100000000000000
    {.size = 512, .offset = 32},     // 32      448     480     0c000000180008016100000000000000
    {.size = 512, .offset = 40},     // 40      448     488     0c000000180008016100000000000000
    {.size = 512, .offset = 47},     // 47      448     495     0c000000180008016100000000000000
    {.size = 512, .offset = 48},     // 48      448     496     0c000000180008016100000000000000
    {.size = 512, .offset = 49},     // 49      424     473     0d000000180008026161000000000000
    {.size = 512, .offset = 50},     // 50      424     474     0d000000180008026161000000000000
    {.size = 512, .offset = 51},     // 51      424     475     0d000000180008026161000000000000
    {.size = 512, .offset = 52},     // 52      424     476     0d000000180008026161000000000000
    {.size = 512, .offset = 53},     // 53      424     477     0d000000180008026161000000000000
    {.size = 512, .offset = 54},     // 54      424     478     0d000000180008026161000000000000
    {.size = 512, .offset = 55},     // 55      424     479     0d000000180008026161000000000000
    {.size = 512, .offset = 56},     // 56      424     480     0d000000180008026161000000000000
    {.size = 512, .offset = 57},     // 57      424     481     0d000000180008026161000000000000
    {.size = 512, .offset = 64},     // 64      424     488     0d000000180008026161000000000000
    {.size = 512, .offset = 128},    // 128     352     480     10000000180008056161616161000000
    {.size = 512, .offset = 512},    // 512     480     992     1b0000002800081066696c656e616d65
    {.size = 513, .offset = 0},      // 0       496     496     07000000180004012e00000000000000
    {.size = 536, .offset = 0},      // 0       496     496     07000000180004012e00000000000000
    {.size = 64, .offset = 1008},    // 1008    0       1008    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1011},    // 1011    0       1011    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1012},    // 1012    0       1012    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1015},    // 1015    0       1015    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1016},    // 1016    0       1016    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1017},    // 1017    0       1017    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1015},    // 1015    0       1015    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1016},    // 1016    0       1016    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1017},    // 1017    0       1017    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 1023, .offset = 0},     // 0       496     496     07000000180004012e00000000000000
    {.size = 1024, .offset = 0},     // 0       1016    1016    07000000180004012e00000000000000
    {.size = 1024, .offset = 511},   // 511     480     991     1b0000002800081066696c656e616d65
    {.size = 1024, .offset = 512},   // 512     1000    1512    1b0000002800081066696c656e616d65
    {.size = 1024, .offset = 513},   // 513     1000    1513    1b0000002800081066696c656e616d65
    {.size = 1025, .offset = 513},   // 513     1000    1513    1b0000002800081066696c656e616d65
    {.size = 1026, .offset = 513},   // 513     1000    1513    1b0000002800081066696c656e616d65
    {.size = 256, .offset = 256},    // 256     240     496     140000002800081066696c656e616d65
    {.size = 23, .offset = 511},     // 511     0       511     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 24, .offset = 511},     // 511     0       511     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 25, .offset = 511},     // 511     0       511     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 8, .offset = 504},      // 504     0       504     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 16, .offset = 496},     // 496     0       496     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 24, .offset = 488},     // 488     0       488     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 48, .offset = 464},     // 464     40      504     1a0000002800081066696c656e616d65
    {.size = 64, .offset = 448},     // 448     40      488     190000002800081066696c656e616d65
    {.size = 128, .offset = 384},    // 384     120     504     180000002800081066696c656e616d65
    {.size = 256, .offset = 256},    // 256     240     496     140000002800081066696c656e616d65
    {.size = 511, .offset = 1},      // 1       472     473     03000000180004022e2e000000000000
    {.size = 32, .offset = 4064},    // 4064    0       4064    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 4064},    // 4064    0       4064    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 4064},    // 4064    0       4064    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 112, .offset = 4064},   // 4064    0       4064    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 128, .offset = 4064},   // 4064    0       4064    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 256, .offset = 4064},   // 4064    0       4064    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 544, .offset = 4064},   // 4064    504     4568    670000003800082066696c6577697468
    {.size = 1024, .offset = 0},     // 0       1016    1016    07000000180004012e00000000000000
    {.size = 32, .offset = 480},     // 480     0       480     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 1024, .offset = 10000}, // 10000   648     65536   d30000004800083466696c6577697468
    {.size = 8192, .offset = 10000}, // 10000   648     65536   d30000004800083466696c6577697468
    {.size = 8192, .offset = 35565}, // 35565   0       65536   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 511, .offset = 1024},   // 170     EINVAL  1024    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 128, .offset = 4096},   // 170     EINVAL  4096    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    //
    {.size = 0xFFFFFFFFFFFFFFFF, .offset = 0}, //
    {.size = 0xFFFFFFFFFFFFFFFE, .offset = 0}, //
    {.size = 0xFFFFFFFFFFFFFF, .offset = 0},   //
    {.size = 0xFFFFFFFFFFFFFE, .offset = 0},   //
    {.size = 0xFFFFFFFFFFFF, .offset = 0},     //
    {.size = 0xFFFFFFFFFFFE, .offset = 0},     //
    {.size = 0xFFFFFFFFFF, .offset = 0},       //
    {.size = 0xFFFFFFFFFE, .offset = 0},       //
    {.size = 0xFFFFFFFF, .offset = 0},         //
    {.size = 0xFFFFFFFE, .offset = 0},         //
    {.size = 0xEFFFFFFF, .offset = 0},         //
    {.size = 0xEFFFFFFE, .offset = 0},         //
    {.size = 0xDFFFFFFF, .offset = 0},         //
    {.size = 0xDFFFFFFE, .offset = 0},         //
    {.size = 0xCFFFFFFF, .offset = 0},         //
    {.size = 0xCFFFFFFE, .offset = 0},         //
    {.size = 0xBFFFFFFF, .offset = 0},         //
    {.size = 0xBFFFFFFE, .offset = 0},         //
    {.size = 0xAFFFFFFF, .offset = 0},         //
    {.size = 0xAFFFFFFE, .offset = 0},         //
    {.size = 0xA0000000, .offset = 0},         //
    {.size = 0x9FFFFFFF, .offset = 0},         //
    {.size = 0x9FFFFFFE, .offset = 0},         //
    {.size = 0x90000000, .offset = 0},         //
    {.size = 0x8FFFFFFF, .offset = 0},         //
    {.size = 0x8FFFFFFE, .offset = 0},         //
    {.size = 0x80000000, .offset = 0},         //
    {.size = 0x7FFFFFFF, .offset = 0},         //
    {.size = 0x7FFFFFFE, .offset = 0},         //
    {.size = 0x70000000, .offset = 0},         //
    {.size = 0x6FFFFFFF, .offset = 0},         //
    {.size = 0x6FFFFFFE, .offset = 0},         //
    {.size = 0xFFFFFF, .offset = 0},           //
    {.size = 0xFFFFFE, .offset = 0},           //
    {.size = 0xFFFF, .offset = 0},             //
    {.size = 0xFFFE, .offset = 0},             //
    {.size = 0xFF, .offset = 0},               //
    {.size = 0xFE, .offset = 0},               //
};
