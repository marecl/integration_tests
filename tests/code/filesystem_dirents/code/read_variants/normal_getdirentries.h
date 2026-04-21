#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::spec_t> normal_dirent_variants = {
    // {.size = 0, .offset = 0},        // 0       EINVAL  0       aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 128, .offset = 128},    // 0       EINVAL  128     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 1024}, // 1024    512     1536    12d50c011c00081066696c656e616d65
    // {.size = 128, .offset = 128},    // 1024    EINVAL  128     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 0},      // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 512, .offset = -1},     // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 512, .offset = -1},     // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 512, .offset = 16},     // 16      496     512     0c0004022e2e000008d40c010c000801
    {.size = 512, .offset = 24},     // 24      488     512     08d40c010c000801610000001ed40c01
    {.size = 512, .offset = 28},     // 28      484     512     0c000801610000001ed40c010c000802
    {.size = 512, .offset = 32},     // 32      480     512     610000001ed40c010c00080261610000
    {.size = 512, .offset = 40},     // 40      472     512     0c000802616100001fd40c010c000803
    {.size = 512, .offset = 47},     // 47      465     512     001fd40c010c000803616161002ed40c
    {.size = 512, .offset = 48},     // 48      464     512     1fd40c010c000803616161002ed40c01
    {.size = 512, .offset = 49},     // 49      463     512     d40c010c000803616161002ed40c0110
    {.size = 512, .offset = 50},     // 50      462     512     0c010c000803616161002ed40c011000
    {.size = 512, .offset = 51},     // 51      461     512     010c000803616161002ed40c01100008
    {.size = 512, .offset = 52},     // 52      460     512     0c000803616161002ed40c0110000804
    {.size = 512, .offset = 53},     // 53      459     512     000803616161002ed40c011000080461
    {.size = 512, .offset = 54},     // 54      458     512     0803616161002ed40c01100008046161
    {.size = 512, .offset = 55},     // 55      457     512     03616161002ed40c0110000804616161
    {.size = 512, .offset = 56},     // 56      456     512     616161002ed40c011000080461616161
    {.size = 512, .offset = 57},     // 57      455     512     6161002ed40c01100008046161616100
    {.size = 512, .offset = 64},     // 64      448     512     100008046161616100000000eed40c01
    {.size = 512, .offset = 128},    // 128     384     512     f1d40c011400080a6161616161616161
    {.size = 512, .offset = 512},    // 512     512     1024    00d50c011c00081066696c656e616d65
    {.size = 513, .offset = 0},      // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 536, .offset = 0},      // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 64, .offset = 1008},    //
    {.size = 64, .offset = 1012},    //
    {.size = 64, .offset = 1015},    // 1015    9       1024    000000000000000000aaaaaaaaaaaaaa
    {.size = 64, .offset = 1016},    // 1016    8       1024    0000000000000000aaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1017},    // 1017    7       1024    00000000000000aaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1015},    // 1015    9       1024    000000000000000000aaaaaaaaaaaaaa
    {.size = 80, .offset = 1016},    // 1016    8       1024    0000000000000000aaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1017},    // 1017    7       1024    00000000000000aaaaaaaaaaaaaaaaaa
    {.size = 1023, .offset = 0},     // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 1023, .offset = -1},    // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 1023, .offset = -1},    // 0       512     512     06d40c010c0004012e00000004d40c01
    {.size = 1024, .offset = 0},     // 0       1024    1024    06d40c010c0004012e00000004d40c01
    {.size = 1024, .offset = 511},   // 511     513     1024    0000d50c011c00081066696c656e616d
    {.size = 1024, .offset = 512},   // 512     1024    1536    00d50c011c00081066696c656e616d65
    {.size = 1024, .offset = 513},   // 513     1023    1536    d50c011c00081066696c656e616d6564
    {.size = 1025, .offset = 513},   // 513     1023    1536    d50c011c00081066696c656e616d6564
    {.size = 1026, .offset = 513},   // 513     1023    1536    d50c011c00081066696c656e616d6564
    {.size = 1026, .offset = -1},    // 513     1023    1536    d50c011c00081066696c656e616d6564
    {.size = 256, .offset = 256},    // 256     256     512     00000000f7d40c011c00081066696c65
    {.size = 256, .offset = -1},     // 256     256     512     00000000f7d40c011c00081066696c65
    {.size = 23, .offset = 511},     // 511     1       512     00aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 24, .offset = 511},     // 511     1       512     00aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 25, .offset = 511},     // 511     1       512     00aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 8, .offset = 504},      // 504     8       512     7874313300000000aaaaaaaaaaaaaaaa
    {.size = 16, .offset = 496},     // 496     16      512     6e616d65646f74657874313300000000
    {.size = 24, .offset = 488},     // 488     24      512     1c00081066696c656e616d65646f7465
    {.size = 48, .offset = 464},     // 464     48      512     66696c656e616d65646f746578743132
    {.size = 64, .offset = 448},     // 448     64      512     7874313100000000fed40c011c000810
    {.size = 128, .offset = 384},    // 384     128     512     6e616d65646f74657874303900000000
    {.size = 256, .offset = 256},    // 256     256     512     00000000f7d40c011c00081066696c65
    {.size = 511, .offset = 1},      // 1       511     512     d40c010c0004012e00000004d40c010c
    {.size = 32, .offset = 4064},    // 4064    32      4096    676e616d653037000000000000000000
    {.size = 64, .offset = 4064},    // 4064    32      4096    676e616d653037000000000000000000
    {.size = 80, .offset = 4064},    // 4064    32      4096    676e616d653037000000000000000000
    {.size = 112, .offset = 4064},   // 4064    32      4096    676e616d653037000000000000000000
    {.size = 128, .offset = 4064},   // 4064    32      4096    676e616d653037000000000000000000
    {.size = 256, .offset = 4064},   // 4064    32      4096    676e616d653037000000000000000000
    {.size = 544, .offset = 4064},   // 4064    544     4608    676e616d653037000000000000000000
    {.size = 1024, .offset = 0},     // 0       1024    1024    06d40c010c0004012e00000004d40c01
    {.size = 1024, .offset = -1},    // 0       1024    1024    06d40c010c0004012e00000004d40c01
    {.size = 32, .offset = 480},     // 480     32      512     00000000ffd40c011c00081066696c65
    {.size = 1024, .offset = 10000}, // 10000   0       10000   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 8192, .offset = 10000}, // 10000   0       10000   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 8192, .offset = 35565}, // 35565   0       35565   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                                     // {.size = 511, .offset = 1024},   // 35565   EINVAL  1024    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                                     // {.size = 128, .offset = 4096},   // 35565   EINVAL  4096    aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
};
