#include "../fs_test.h"

#include <vector>

// read size, read offset // HW return, HW end position
std::vector<OrbisInternals::spec_t> pfs_read_variants = {
    {.size = 0, .offset = 0},             // 0       0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 0},            // 64      64      07000000040000000100000018000000
    {.size = 128, .offset = 0},           // 128     128     07000000040000000100000018000000
    {.size = 256, .offset = 0},           // 256     256     07000000040000000100000018000000
    {.size = 511, .offset = 0},           // 511     511     07000000040000000100000018000000
    {.size = 511, .offset = 1},           // 511     512     0000000400000001000000180000002e
    {.size = 511, .offset = 2},           // 511     513     00000400000001000000180000002e00
    {.size = 512, .offset = 0},           // 512     512     07000000040000000100000018000000
    {.size = 2048, .offset = 1245},       // 2048    3293    74323500000000000000002d00000002
    {.size = 7257, .offset = 1245},       // 7257    8502    74323500000000000000002d00000002
    {.size = 418, .offset = 574},         // 418     992     00001c00000002000000100000002800
    {.size = 9363, .offset = 1111},       // 9363    10474   0066696c656e616d65646f7465787432
    {.size = 37865, .offset = 936},       // 37865   38801   25000000020000001000000028000000
    {.size = 17543, .offset = 1245},      // 17543   18788   74323500000000000000002d00000002
    {.size = 1024, .offset = 35565},      // 1024    36589   00000000000000000000000000000000
    {.size = 512, .offset = 65534},       // 2       65536   0000aaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 2048, .offset = 65534},      // 2       65536   0000aaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 8192},       // 4096    12288   746865766572796f6e65313600000000
    {.size = 1024, .offset = 8000},       // 1024    9024    340000004800000066696c6577697468
    {.size = 1024, .offset = 10000},      // 1024    11024   6e65343100000000d300000002000000
    {.size = 4096, .offset = 10000},      // 4096    14096   6e65343100000000d300000002000000
    {.size = u64(-1234), .offset = 2345}, // EINVAL  2345    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    //
    {.size = 0xFFFFFFFFFFFFFFFF, .offset = 0}, // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFFFFFFFE, .offset = 0}, // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFFFFFF, .offset = 0},   // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFFFFFE, .offset = 0},   // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFFFF, .offset = 0},     // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFFFE, .offset = 0},     // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFF, .offset = 0},       // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFFFE, .offset = 0},       // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xFFFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xEFFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xEFFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xDFFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xDFFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xCFFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xCFFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xBFFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xBFFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xAFFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xAFFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0xA0000000, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x9FFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x9FFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x90000000, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x8FFFFFFF, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x8FFFFFFE, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x80000000, .offset = 0},         // EINVAL    0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 0x7FFFFFFF, .offset = 0},         // 65536     65536   07000000040000000100000018000000
    {.size = 0x7FFFFFFE, .offset = 0},         // 65536     65536   07000000040000000100000018000000
    {.size = 0x70000000, .offset = 0},         // 65536     65536   07000000040000000100000018000000
    {.size = 0x6FFFFFFF, .offset = 0},         // 65536     65536   07000000040000000100000018000000
    {.size = 0x6FFFFFFE, .offset = 0},         // 65536     65536   07000000040000000100000018000000
    {.size = 0xFFFFFF, .offset = 0},           // 65536     65536   07000000040000000100000018000000
    {.size = 0xFFFFFE, .offset = 0},           // 65536     65536   07000000040000000100000018000000
    {.size = 0xFFFF, .offset = 0},             // 65535     65535   07000000040000000100000018000000
    {.size = 0xFFFE, .offset = 0},             // 65534     65534   07000000040000000100000018000000
    {.size = 0xFF, .offset = 0},               // 255       255     07000000040000000100000018000000
    {.size = 0xFE, .offset = 0},               // 254       254     07000000040000000100000018000000
};
