#ifndef PFS_LSEEK_H
#define PFS_LSEEK_H

#pragma once

#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::offset_spec_t> pfs_lseek_variants = {
    {.offset = 0, .whence = 0},                       // 0                  0                   0   ->  0                   0   ->  0                   0
    {.offset = -123, .whence = 0},                    // 0                  -123                0   ->  EINVAL              22  ->  EINVAL              22
    {.offset = 123456, .whence = 0},                  // 0                  123456              0   ->  123456              0   ->  123456              0
    {.offset = 60, .whence = 0},                      // 0                  60                  0   ->  60                  0   ->  60                  0
    {.offset = 0, .whence = 1},                       // 123456             0                   1   ->  60                  0   ->  60                  0
    {.offset = 24, .whence = 1},                      // 60                 24                  1   ->  84                  0   ->  84                  0
    {.offset = -24, .whence = 1},                     // 60                 -24                 1   ->  60                  0   ->  60                  0
    {.offset = -6666, .whence = 1},                   // 84                 -6666               1   ->  EINVAL              22  ->  EINVAL              22
    {.offset = 123456, .whence = 1},                  // 60                 123456              1   ->  123516              0   ->  123516              0
    {.offset = 0, .whence = 2},                       // 60                 0                   2   ->  65536               0   ->  65536               0
    {.offset = 123456, .whence = 2},                  // 123516             123456              2   ->  188992              0   ->  188992              0
    {.offset = 100, .whence = 2},                     // 65536              100                 2   ->  65636               0   ->  65636               0
    {.offset = -100, .whence = 2},                    // 188992             -100                2   ->  65436               0   ->  65436               0
    {.offset = -100000, .whence = 2},                 // 65636              -100000             2   ->  EINVAL              22  ->  EINVAL              22
    {.offset = -normal_read_target - 1, .whence = 2}, // 65436              -8705               2   ->  56831               0   ->  56831               0
    {.offset = -normal_read_target, .whence = 2},     // 65436              -8704               2   ->  56832               0   ->  56832               0
    {.offset = -normal_read_target + 1, .whence = 2}, // 56831              -8703               2   ->  56833               0   ->  56833               0
    {.offset = 0, .whence = 3},                       // 56832              0                   3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 8, .whence = 3},                       // 56833              8                   3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 16, .whence = 3},                      // 56833              16                  3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 24, .whence = 3},                      // 56833              24                  3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 32, .whence = 3},                      // 56833              32                  3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 40, .whence = 3},                      // 56833              40                  3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 10240, .whence = 3},                   // 56833              10240               3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -1024, .whence = 3},                   // 56833              -1024               3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -10240, .whence = 3},                  // 56833              -10240              3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -pfs_read_target - 1, .whence = 3},    // 56833              -10657              3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -pfs_read_target, .whence = 3},        // 56833              -10656              3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -pfs_read_target + 1, .whence = 3},    // 56833              -10655              3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -normal_read_target - 1, .whence = 3}, // 56833              -8705               3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -normal_read_target, .whence = 3},     // 56833              -8704               3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -normal_read_target + 1, .whence = 3}, // 56833              -8703               3   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 0, .whence = 4},                       // 56833              0                   4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 8, .whence = 4},                       // 56833              8                   4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 16, .whence = 4},                      // 56833              16                  4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 24, .whence = 4},                      // 56833              24                  4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 32, .whence = 4},                      // 56833              32                  4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 40, .whence = 4},                      // 56833              40                  4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = 10240, .whence = 4},                   // 56833              10240               4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -1024, .whence = 4},                   // 56833              -1024               4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -10240, .whence = 4},                  // 56833              -10240              4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -pfs_read_target - 1, .whence = 4},    // 56833              -10657              4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -pfs_read_target, .whence = 4},        // 56833              -10656              4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -pfs_read_target + 1, .whence = 4},    // 56833              -10655              4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -normal_read_target - 1, .whence = 4}, // 56833              -8705               4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -normal_read_target, .whence = 4},     // 56833              -8704               4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = -normal_read_target + 1, .whence = 4}, // 56833              -8703               4   ->  ENOTTY              25  ->  ENOTTY              25
    {.offset = s64(0xFFFFFFFFFFFFFFFF), .whence = 0}, // 56833              -1                  0   ->  EINVAL              22  ->  EINVAL              22
    {.offset = s64(0xFFFFFFFFFFFFFFFE), .whence = 0}, // 56833              -2                  0   ->  EINVAL              22  ->  EINVAL              22
    {.offset = 0xFFFFFFFFFFFFFF, .whence = 0},        // 56833              72057594037927935   0   ->  72057594037927935   0   ->  72057594037927935   0
    {.offset = 0xFFFFFFFFFFFFFE, .whence = 0},        // 56833              72057594037927934   0   ->  72057594037927934   0   ->  72057594037927934   0
    {.offset = 0xFFFFFFFFFFFF, .whence = 0},          // 72057594037927935  281474976710655     0   ->  281474976710655     0   ->  281474976710655     0
    {.offset = 0xFFFFFFFFFFFE, .whence = 0},          // 72057594037927934  281474976710654     0   ->  281474976710654     0   ->  281474976710654     0
    {.offset = 0xFFFFFFFFFF, .whence = 0},            // 281474976710655    1099511627775       0   ->  1099511627775       0   ->  1099511627775       0
    {.offset = 0xFFFFFFFFFE, .whence = 0},            // 281474976710654    1099511627774       0   ->  1099511627774       0   ->  1099511627774       0
    {.offset = 0xFFFFFFFF, .whence = 0},              // 1099511627775      4294967295          0   ->  4294967295          0   ->  4294967295          0
    {.offset = 0xFFFFFFFE, .whence = 0},              // 1099511627774      4294967294          0   ->  4294967294          0   ->  4294967294          0
    {.offset = 0xEFFFFFFF, .whence = 0},              // 4294967295         4026531839          0   ->  4026531839          0   ->  4026531839          0
    {.offset = 0xEFFFFFFE, .whence = 0},              // 4294967294         4026531838          0   ->  4026531838          0   ->  4026531838          0
    {.offset = 0xDFFFFFFF, .whence = 0},              // 4026531839         3758096383          0   ->  3758096383          0   ->  3758096383          0
    {.offset = 0xDFFFFFFE, .whence = 0},              // 4026531838         3758096382          0   ->  3758096382          0   ->  3758096382          0
    {.offset = 0xCFFFFFFF, .whence = 0},              // 3758096383         3489660927          0   ->  3489660927          0   ->  3489660927          0
    {.offset = 0xCFFFFFFE, .whence = 0},              // 3758096382         3489660926          0   ->  3489660926          0   ->  3489660926          0
    {.offset = 0xBFFFFFFF, .whence = 0},              // 3489660927         3221225471          0   ->  3221225471          0   ->  3221225471          0
    {.offset = 0xBFFFFFFE, .whence = 0},              // 3489660926         3221225470          0   ->  3221225470          0   ->  3221225470          0
    {.offset = 0xAFFFFFFF, .whence = 0},              // 3221225471         2952790015          0   ->  2952790015          0   ->  2952790015          0
    {.offset = 0xAFFFFFFE, .whence = 0},              // 3221225470         2952790014          0   ->  2952790014          0   ->  2952790014          0
    {.offset = 0xA0000000, .whence = 0},              // 2952790015         2684354560          0   ->  2684354560          0   ->  2684354560          0
    {.offset = 0x9FFFFFFF, .whence = 0},              // 2952790014         2684354559          0   ->  2684354559          0   ->  2684354559          0
    {.offset = 0x9FFFFFFE, .whence = 0},              // 2684354560         2684354558          0   ->  2684354558          0   ->  2684354558          0
    {.offset = 0x90000000, .whence = 0},              // 2684354559         2415919104          0   ->  2415919104          0   ->  2415919104          0
    {.offset = 0x8FFFFFFF, .whence = 0},              // 2684354558         2415919103          0   ->  2415919103          0   ->  2415919103          0
    {.offset = 0x8FFFFFFE, .whence = 0},              // 2415919104         2415919102          0   ->  2415919102          0   ->  2415919102          0
    {.offset = 0x80000000, .whence = 0},              // 2415919103         2147483648          0   ->  2147483648          0   ->  2147483648          0
    {.offset = 0x7FFFFFFF, .whence = 0},              // 2415919102         2147483647          0   ->  2147483647          0   ->  2147483647          0
    {.offset = 0x7FFFFFFE, .whence = 0},              // 2147483648         2147483646          0   ->  2147483646          0   ->  2147483646          0
    {.offset = 0x70000000, .whence = 0},              // 2147483647         1879048192          0   ->  1879048192          0   ->  1879048192          0
    {.offset = 0x6FFFFFFF, .whence = 0},              // 2147483646         1879048191          0   ->  1879048191          0   ->  1879048191          0
    {.offset = 0x6FFFFFFE, .whence = 0},              // 1879048192         1879048190          0   ->  1879048190          0   ->  1879048190          0
    {.offset = 0xFFFFFF, .whence = 0},                // 1879048191         16777215            0   ->  16777215            0   ->  16777215            0
    {.offset = 0xFFFFFE, .whence = 0},                // 1879048190         16777214            0   ->  16777214            0   ->  16777214            0
    {.offset = 0xFFFF, .whence = 0},                  // 16777215           65535               0   ->  65535               0   ->  65535               0
    {.offset = 0xFFFE, .whence = 0},                  // 16777214           65534               0   ->  65534               0   ->  65534               0
    {.offset = 0xFF, .whence = 0},                    // 65535              255                 0   ->  255                 0   ->  255                 0
    {.offset = 0xFE, .whence = 0},                    // 65534              254                 0   ->  254                 0   ->  254                 0
};

#endif // PFS_LSEEK_H
