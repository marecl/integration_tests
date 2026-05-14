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

    {.size = 0, .offset = 0},        // 0       0       ->  1       (0)     EINVAL  0       22  ->  170     EINVAL  0       22  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 128, .offset = 128},    // 128     128     ->  1       (128)   EINVAL  128     22  ->  170     EINVAL  128     22  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 1024},   // 512     1024    ->  1024    (1056)  480     1504    0   ->  1024    480     1504    0   280000002800081066696c656e616d65
    {.size = 128, .offset = 128},    // 128     128     ->  1       (128)   EINVAL  128     22  ->  170     EINVAL  128     22  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 512, .offset = 0},      // 512     0       ->  0       (0)     496     496     0   ->  0       496     496     0   07000000180004012e00000000000000
    {.size = 512, .offset = 16},     // 512     16      ->  16      (24)    472     488     0   ->  16      472     488     0   03000000180004022e2e000000000000
    {.size = 512, .offset = 24},     // 512     24      ->  24      (24)    472     496     0   ->  24      472     496     0   03000000180004022e2e000000000000
    {.size = 512, .offset = 28},     // 512     28      ->  28      (48)    448     476     0   ->  28      448     476     0   0c000000180008016100000000000000
    {.size = 512, .offset = 32},     // 512     32      ->  32      (48)    448     480     0   ->  32      448     480     0   0c000000180008016100000000000000
    {.size = 512, .offset = 40},     // 512     40      ->  40      (48)    448     488     0   ->  40      448     488     0   0c000000180008016100000000000000
    {.size = 512, .offset = 47},     // 512     47      ->  47      (48)    448     495     0   ->  47      448     495     0   0c000000180008016100000000000000
    {.size = 512, .offset = 48},     // 512     48      ->  48      (48)    448     496     0   ->  48      448     496     0   0c000000180008016100000000000000
    {.size = 512, .offset = 49},     // 512     49      ->  49      (72)    424     473     0   ->  49      424     473     0   0d000000180008026161000000000000
    {.size = 512, .offset = 50},     // 512     50      ->  50      (72)    424     474     0   ->  50      424     474     0   0d000000180008026161000000000000
    {.size = 512, .offset = 51},     // 512     51      ->  51      (72)    424     475     0   ->  51      424     475     0   0d000000180008026161000000000000
    {.size = 512, .offset = 52},     // 512     52      ->  52      (72)    424     476     0   ->  52      424     476     0   0d000000180008026161000000000000
    {.size = 512, .offset = 53},     // 512     53      ->  53      (72)    424     477     0   ->  53      424     477     0   0d000000180008026161000000000000
    {.size = 512, .offset = 54},     // 512     54      ->  54      (72)    424     478     0   ->  54      424     478     0   0d000000180008026161000000000000
    {.size = 512, .offset = 55},     // 512     55      ->  55      (72)    424     479     0   ->  55      424     479     0   0d000000180008026161000000000000
    {.size = 512, .offset = 56},     // 512     56      ->  56      (72)    424     480     0   ->  56      424     480     0   0d000000180008026161000000000000
    {.size = 512, .offset = 57},     // 512     57      ->  57      (72)    424     481     0   ->  57      424     481     0   0d000000180008026161000000000000
    {.size = 512, .offset = 64},     // 512     64      ->  64      (72)    424     488     0   ->  64      424     488     0   0d000000180008026161000000000000
    {.size = 512, .offset = 128},    // 512     128     ->  128     (144)   352     480     0   ->  128     352     480     0   10000000180008056161616161000000
    {.size = 512, .offset = 512},    // 512     512     ->  512     (536)   480     992     0   ->  512     480     992     0   1b0000002800081066696c656e616d65
    {.size = 513, .offset = 0},      // 513     0       ->  0       (0)     496     496     0   ->  0       496     496     0   07000000180004012e00000000000000
    {.size = 536, .offset = 0},      // 536     0       ->  0       (0)     496     496     0   ->  0       496     496     0   07000000180004012e00000000000000
    {.size = 64, .offset = 1008},    // 64      1008    ->  1008    (1016)  0       1008    0   ->  1008    0       1008    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1011},    // 64      1011    ->  1011    (1016)  0       1011    0   ->  1011    0       1011    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1012},    // 64      1012    ->  1012    (1016)  0       1012    0   ->  1012    0       1012    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1015},    // 64      1015    ->  1015    (1016)  0       1015    0   ->  1015    0       1015    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1016},    // 64      1016    ->  1016    (1016)  0       1016    0   ->  1016    0       1016    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 1017},    // 64      1017    ->  1017    (1056)  0       1017    0   ->  1017    0       1017    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1015},    // 80      1015    ->  1015    (1016)  0       1015    0   ->  1015    0       1015    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1016},    // 80      1016    ->  1016    (1016)  0       1016    0   ->  1016    0       1016    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 1017},    // 80      1017    ->  1017    (1056)  0       1017    0   ->  1017    0       1017    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 1023, .offset = 0},     // 1023    0       ->  0       (0)     496     496     0   ->  0       496     496     0   07000000180004012e00000000000000
    {.size = 1024, .offset = 0},     // 1024    0       ->  0       (0)     1016    1016    0   ->  0       1016    1016    0   07000000180004012e00000000000000
    {.size = 1024, .offset = 511},   // 1024    511     ->  511     (536)   480     991     0   ->  511     480     991     0   1b0000002800081066696c656e616d65
    {.size = 1024, .offset = 512},   // 1024    512     ->  512     (536)   1000    1512    0   ->  512     1000    1512    0   1b0000002800081066696c656e616d65
    {.size = 1024, .offset = 513},   // 1024    513     ->  513     (536)   1000    1513    0   ->  513     1000    1513    0   1b0000002800081066696c656e616d65
    {.size = 1025, .offset = 513},   // 1025    513     ->  513     (536)   1000    1513    0   ->  513     1000    1513    0   1b0000002800081066696c656e616d65
    {.size = 1026, .offset = 513},   // 1026    513     ->  513     (536)   1000    1513    0   ->  513     1000    1513    0   1b0000002800081066696c656e616d65
    {.size = 256, .offset = 256},    // 256     256     ->  256     (256)   240     496     0   ->  256     240     496     0   140000002800081066696c656e616d65
    {.size = 23, .offset = 511},     // 23      511     ->  511     (536)   0       511     0   ->  511     0       511     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 24, .offset = 511},     // 24      511     ->  511     (536)   0       511     0   ->  511     0       511     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 25, .offset = 511},     // 25      511     ->  511     (536)   0       511     0   ->  511     0       511     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 8, .offset = 504},      // 8       504     ->  504     (536)   0       504     0   ->  504     0       504     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 16, .offset = 496},     // 16      496     ->  496     (496)   0       496     0   ->  496     0       496     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 24, .offset = 488},     // 24      488     ->  488     (496)   0       488     0   ->  488     0       488     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 48, .offset = 464},     // 48      464     ->  464     (496)   40      504     0   ->  464     40      504     0   1a0000002800081066696c656e616d65
    {.size = 64, .offset = 448},     // 64      448     ->  448     (456)   40      488     0   ->  448     40      488     0   190000002800081066696c656e616d65
    {.size = 128, .offset = 384},    // 128     384     ->  384     (416)   120     504     0   ->  384     120     504     0   180000002800081066696c656e616d65
    {.size = 256, .offset = 256},    // 256     256     ->  256     (256)   240     496     0   ->  256     240     496     0   140000002800081066696c656e616d65
    {.size = 511, .offset = 1},      // 511     1       ->  1       (24)    472     473     0   ->  1       472     473     0   03000000180004022e2e000000000000
    {.size = 32, .offset = 4064},    // 32      4064    ->  4064    (4104)  0       4064    0   ->  4064    0       4064    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 64, .offset = 4064},    // 64      4064    ->  4064    (4104)  0       4064    0   ->  4064    0       4064    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 80, .offset = 4064},    // 80      4064    ->  4064    (4104)  0       4064    0   ->  4064    0       4064    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 112, .offset = 4064},   // 112     4064    ->  4064    (4104)  0       4064    0   ->  4064    0       4064    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 128, .offset = 4064},   // 128     4064    ->  4064    (4104)  0       4064    0   ->  4064    0       4064    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 256, .offset = 4064},   // 256     4064    ->  4064    (4104)  0       4064    0   ->  4064    0       4064    0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 544, .offset = 4064},   // 544     4064    ->  4064    (4104)  504     4568    0   ->  4064    504     4568    0   670000003800082066696c6577697468
    {.size = 1024, .offset = 0},     // 1024    0       ->  0       (0)     1016    1016    0   ->  0       1016    1016    0   07000000180004012e00000000000000
    {.size = 32, .offset = 480},     // 32      480     ->  480     (496)   0       480     0   ->  480     0       480     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 1024, .offset = 10000}, // 1024    10000   ->  10000   (10008) 648     65536   0   ->  10000   648     65536   0   d30000004800083466696c6577697468
    {.size = 8192, .offset = 10000}, // 8192    10000   ->  10000   (10008) 648     65536   0   ->  10000   648     65536   0   d30000004800083466696c6577697468
    {.size = 8192, .offset = 35565}, // 8192    35565   ->  35565   (35565) 0       65536   0   ->  35565   0       65536   0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 511, .offset = 1024},   // 511     1024    ->  1       (1024)  EINVAL  1024    22  ->  170     EINVAL  1024    22  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    {.size = 128, .offset = 4096},   // 128     4096    ->  1       (4096)  EINVAL  4096    22  ->  170     EINVAL  4096    22  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

    {.size = 1024, .offset = 65000}, //
    {.size = 1024, .offset = 65023}, //
    {.size = 1024, .offset = 65024}, //
    {.size = 1024, .offset = 65025}, //
    {.size = 1024, .offset = 70123}, //
    {.size = 1024, .offset = 92616}, //
    {.size = 1024, .offset = 92544}, //

    // there is literally no point in testing those
    // some of those cause something somewhere which may (or may not) crash the console
    // for some reason only PFS has issues with these tests
    // {.size = 1024, .offset = (s64(0xFF) << 0)},  // 1024 255 -> 255 (256) 760 1015 0 -> 255 760 1015 0 140000002800081066696c656e616d65
    // {.size = 1024, .offset = (s64(0xFF) << 8)},  // 1024 65280 -> 65280 (65280) 0 65536 0 -> 65280 0 65536 0 aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 1024, .offset = (s64(0xFF) << 16)}, // 1024 16711680 -> 16711680 (16711680) 0 16711680 0 -> 16711680 0 16711680 0
    // aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 1024, .offset = (s64(0xFF) << 24)}, // 1024    4278190080          -> 4278190080 (4278190080)    0   4278190080  0 ->
    //                                              // 4278190080          0 4278190080        0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 1024, .offset = (s64(0xFF) << 32)}, // 1024    1095216660480       -> 1095216660480 (1095216660480) 0 1095216660480 0 ->
    //                                              // 1095216660480       0 1095216660480     0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 1024, .offset = (s64(0xFF) << 40)}, // 1024    280375465082880     -> 280375465082880 (280375465082880) 0 280375465082880 0 ->
    //                                              // 280375465082880     0 280375465082880   0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 1024, .offset = (s64(0xFF) << 48)}, // 1024    71776119061217280   -> 71776119061217280 (71776119061217280) 0 71776119061217280 0 ->
    //                                              // 71776119061217280   0 71776119061217280 0   aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    // {.size = 1024, .offset = (s64(0xFF) << 56)}, // 1024    -72057594037927936  -> 0 (0) 1016 1016 0 -> 0 1016 1016 0 07000000180004012e00000000000000
};
