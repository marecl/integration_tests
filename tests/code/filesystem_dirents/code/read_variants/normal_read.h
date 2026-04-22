#include "../fs_test.h"

#include <vector>

// read size, read offset // HW return, HW end position
std::vector<OrbisInternals::spec_t> normal_read_variants = {
    {.size = 0, .offset = 0},         // 0        0          aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 0},        // 64       64         06d40c010c0004012e00000004d40c01
    {.size = 128, .offset = 0},       // 128      128        06d40c010c0004012e00000004d40c01
    {.size = 256, .offset = 0},       // 256      256        06d40c010c0004012e00000004d40c01
    {.size = 511, .offset = 0},       // 511      511        06d40c010c0004012e00000004d40c01
    {.size = 511, .offset = 1},       // 511      512        d40c010c0004012e00000004d40c010c
    {.size = 511, .offset = 2},       // 511      513        0c010c0004012e00000004d40c010c00
    {.size = 512, .offset = 0},       // 512      512        06d40c010c0004012e00000004d40c01
    {.size = 2048, .offset = 1245},   // 2048     3293       0000001ad50c011c00081066696c656e
    {.size = 7257, .offset = 1245},   // 7257     8502       0000001ad50c011c00081066696c656e
    {.size = 418, .offset = 574},     // 418      992        081066696c656e616d65646f74657874
    {.size = 9363, .offset = 1111},   // 7593     8704       011c00081066696c656e616d65646f74
    {.size = 37865, .offset = 936},   // 7768     8704       1c00081066696c656e616d65646f7465
    {.size = 17543, .offset = 1245},  // 7459     8704       0000001ad50c011c00081066696c656e
    {.size = 1024, .offset = 35565},  // 0        35565      aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 65534},   // 0        65534      aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 2048, .offset = 65534},  // 0        65534      aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 8192},   // 512      8704       b3d50c014000083466696c6577697468
    {.size = 1024, .offset = 8000},   // 704      8704       b0d50c014000083466696c6577697468
    {.size = 1024, .offset = 10000},  // 0        10000      aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 10000},  // 0        10000      aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = -1234, .offset = 2345},  // EINVAL   2345   	 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 1234, .offset = -2345},  // 1234     1234   	 04d40c010c0004012e00000003d40c01
    {.size = -1234, .offset = -2345}, // EINVAL   0  	     aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
};
