#include "../fs_test.h"

#include <vector>

std::vector<OrbisInternals::DirentCombinationGetdirentries> normal_dirent_variants = {

    /**
    What i (again) think is that the entire 512byte buffer must be iterated? or sth
    read can occur as long as read descriptor passes 512aligned mark, i.e.
    it will read a couple of bytes if end position falls couple of bytes after the end
    for example read 8@510 offset will read 2 bytes
    2 bytes, because thet's the farthest the current sector can go
    however this is a bit different for larger reads. large reads still have this condition, but they include every buffer on their way

    so the only way this can read stuff is when end position crosses current sector border
    otherwise EINVAL
    */

    {.read_size = 513, .read_offset = 0, .expected_basep = 0, .expected_result = 512, .expected_end_position = 512},
    {.read_size = 512, .read_offset = 0, .expected_basep = 0, .expected_result = 512, .expected_end_position = 512},     //
    {.read_size = 512, .read_offset = -1, .expected_basep = 512, .expected_result = 512, .expected_end_position = 1024}, //
    {.read_size = 1024, .read_offset = 0, .expected_basep = 0, .expected_result = 1024, .expected_end_position = 1024},
    {.read_size = 1024, .read_offset = 511, .expected_basep = 511, .expected_result = 513, .expected_end_position = 1024},
    {.read_size = 1024, .read_offset = 512, .expected_basep = 512, .expected_result = 1024, .expected_end_position = 1536},
    {.read_size = 1024, .read_offset = 513, .expected_basep = 513, .expected_result = 1023, .expected_end_position = 1536},
    {.read_size = 1025, .read_offset = 513, .expected_basep = 513, .expected_result = 1023, .expected_end_position = 1536},
    {.read_size = 1026, .read_offset = 513, .expected_basep = 513, .expected_result = 1023, .expected_end_position = 1536},
    {.read_size = 8, .read_offset = 504, .expected_basep = 504, .expected_result = 8, .expected_end_position = 512},
    {.read_size = 16, .read_offset = 496, .expected_basep = 496, .expected_result = 16, .expected_end_position = 512},
    {.read_size = 24, .read_offset = 488, .expected_basep = 488, .expected_result = 24, .expected_end_position = 512},
    {.read_size = 48, .read_offset = 464, .expected_basep = 464, .expected_result = 48, .expected_end_position = 512},
    {.read_size = 64, .read_offset = 448, .expected_basep = 448, .expected_result = 64, .expected_end_position = 512},
    {.read_size = 128, .read_offset = 384, .expected_basep = 384, .expected_result = 128, .expected_end_position = 512},
    {.read_size = 256, .read_offset = 256, .expected_basep = 256, .expected_result = 256, .expected_end_position = 512},
    {.read_size = 511, .read_offset = 1, .expected_basep = 1, .expected_result = 511, .expected_end_position = 512},
    {.read_size = 32, .read_offset = 4064, .expected_basep = 4064, .expected_result = 32, .expected_end_position = 4096},
    {.read_size = 64, .read_offset = 4064, .expected_basep = 4064, .expected_result = 32, .expected_end_position = 4096},
    {.read_size = 80, .read_offset = 4064, .expected_basep = 4064, .expected_result = 32, .expected_end_position = 4096},
    {.read_size = 128, .read_offset = 4064, .expected_basep = 4064, .expected_result = 32, .expected_end_position = 4096},
    {.read_size = 256, .read_offset = 4064, .expected_basep = 4064, .expected_result = 32, .expected_end_position = 4096},
    {.read_size = 1024, .read_offset = 0, .expected_basep = 0, .expected_result = 1024, .expected_end_position = 1024},
    {.read_size = 1024, .read_offset = -1, .expected_basep = 1024, .expected_result = 1024, .expected_end_position = 2048},
    {.read_size = 1024, .read_offset = -1, .expected_basep = 2048, .expected_result = 1024, .expected_end_position = 3072},
    {.read_size = 32, .read_offset = 480, .expected_basep = 480, .expected_result = 32, .expected_end_position = 512},
};
