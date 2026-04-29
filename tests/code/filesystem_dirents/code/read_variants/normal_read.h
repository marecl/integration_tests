#include "../fs_test.h"

#include <vector>

// read size, read offset // HW return, HW end position
std::vector<OrbisInternals::spec_t> normal_read_variants = {
    {.size = 0, .offset = 0},             // 0       0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 0},            // 64      64      1cd40c010c0004012e00000019d40c01
    {.size = 128, .offset = 0},           // 128     128     1cd40c010c0004012e00000019d40c01
    {.size = 256, .offset = 0},           // 256     256     1cd40c010c0004012e00000019d40c01
    {.size = 511, .offset = 0},           // 511     511     1cd40c010c0004012e00000019d40c01
    {.size = 511, .offset = 1},           // 511     512     d40c010c0004012e00000019d40c010c
    {.size = 511, .offset = 2},           // 511     513     0c010c0004012e00000019d40c010c00
    {.size = 512, .offset = 0},           // 512     512     1cd40c010c0004012e00000019d40c01
    {.size = 2048, .offset = 1245},       // 2048    3293    00000050d40c011c00081066696c656e
    {.size = 7257, .offset = 1245},       // 7257    8502    00000050d40c011c00081066696c656e
    {.size = 418, .offset = 574},         // 418     992     081066696c656e616d65646f74657874
    {.size = 9363, .offset = 1111},       // 7593    8704    011c00081066696c656e616d65646f74
    {.size = 37865, .offset = 936},       // 7768    8704    1c00081066696c656e616d65646f7465
    {.size = 17543, .offset = 1245},      // 7459    8704    00000050d40c011c00081066696c656e
    {.size = 1024, .offset = 35565},      // 0       35565   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 65534},       // 0       65534   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 2048, .offset = 65534},      // 0       65534   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 8192},       // 512     8704    ead40c014000083466696c6577697468
    {.size = 1024, .offset = 8000},       // 704     8704    e7d40c014000083466696c6577697468
    {.size = 1024, .offset = 10000},      // 0       10000   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 10000},      // 0       10000   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
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
    {.size = 0x7FFFFFFF, .offset = 0},         // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0x7FFFFFFE, .offset = 0},         // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0x70000000, .offset = 0},         // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0x6FFFFFFF, .offset = 0},         // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0x6FFFFFFE, .offset = 0},         // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0xFFFFFF, .offset = 0},           // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0xFFFFFE, .offset = 0},           // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0xFFFF, .offset = 0},             // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0xFFFE, .offset = 0},             // 8704      8704    1ed40c010c0004012e0000001dd40c01
    {.size = 0xFF, .offset = 0},               // 255       255     1ed40c010c0004012e0000001dd40c01
    {.size = 0xFE, .offset = 0},               // 254       254     1ed40c010c0004012e0000001dd40c01
};
