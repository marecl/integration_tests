#include "../fs_test.h"

#include <vector>

// read size, read offset // HW return, HW end position
std::vector<OrbisInternals::spec_t> normal_read_variants = {
    {.size = 0, .offset = 0},        // 0       0       ->  0       0       0       ->  0       0       0       ->  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 0},       // 64      0       ->  0       64      64      ->  0       64      64      ->  dad50c010c0004012e000000d8d50c01
    {.size = 128, .offset = 0},      // 128     0       ->  0       128     128     ->  0       128     128     ->  dad50c010c0004012e000000d8d50c01
    {.size = 256, .offset = 0},      // 256     0       ->  0       256     256     ->  0       256     256     ->  dad50c010c0004012e000000d8d50c01
    {.size = 511, .offset = 0},      // 511     0       ->  0       511     511     ->  0       511     511     ->  dad50c010c0004012e000000d8d50c01
    {.size = 511, .offset = 1},      // 511     1       ->  1       511     512     ->  1       511     512     ->  d50c010c0004012e000000d8d50c010c
    {.size = 511, .offset = 2},      // 511     2       ->  2       511     513     ->  2       511     513     ->  0c010c0004012e000000d8d50c010c00
    {.size = 512, .offset = 0},      // 512     0       ->  0       512     512     ->  0       512     512     ->  dad50c010c0004012e000000d8d50c01
    {.size = 2048, .offset = 1245},  // 2048    1245    ->  1245    2048    3293    ->  1245    2048    3293    ->  000000e2d60c011c00081066696c656e
    {.size = 7257, .offset = 1245},  // 7257    1245    ->  1245    7257    8502    ->  1245    7257    8502    ->  000000e2d60c011c00081066696c656e
    {.size = 418, .offset = 574},    // 418     574     ->  574     418     992     ->  574     418     992     ->  081066696c656e616d65646f74657874
    {.size = 9363, .offset = 1111},  // 9363    1111    ->  1111    7593    8704    ->  1111    7593    8704    ->  011c00081066696c656e616d65646f74
    {.size = 37865, .offset = 936},  // 37865   936     ->  936     7768    8704    ->  936     7768    8704    ->  1c00081066696c656e616d65646f7465
    {.size = 17543, .offset = 1245}, // 17543   1245    ->  1245    7459    8704    ->  1245    7459    8704    ->  000000e2d60c011c00081066696c656e
    {.size = 1024, .offset = 35565}, // 1024    35565   ->  35565   0       35565   ->  35565   0       35565   ->  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 65534},  // 512     65534   ->  65534   0       65534   ->  65534   0       65534   ->  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 2048, .offset = 65534}, // 2048    65534   ->  65534   0       65534   ->  65534   0       65534   ->  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 8192},  // 4096    8192    ->  8192    512     8704    ->  8192    512     8704    ->  7bd70c014000083466696c6577697468
    {.size = 1024, .offset = 8000},  // 1024    8000    ->  8000    704     8704    ->  8000    704     8704    ->  78d70c014000083466696c6577697468
    {.size = 1024, .offset = 10000}, // 1024    10000   ->  10000   0       10000   ->  10000   0       10000   ->  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 4096, .offset = 10000}, // 4096    10000   ->  10000   0       10000   ->  10000   0       10000   ->  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

    {.size = 1024, .offset = 65000}, //
    {.size = 1024, .offset = 65023}, //
    {.size = 1024, .offset = 65024}, //
    {.size = 1024, .offset = 65025}, //
    {.size = 1024, .offset = 70123}, //
    {.size = 1024, .offset = 92616}, //
    {.size = 1024, .offset = 92544}, //
};
