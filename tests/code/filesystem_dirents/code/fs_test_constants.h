#ifndef FS_TEST_CONST_H
#define FS_TEST_CONST_H

#pragma once

#define FUZZ_MAX_ITERATIONS 200000 //arbitrary but large
#define FUZZ_MAX_FAILURES   20

constexpr s64 files_cloned_target         = 208;
constexpr s64 files_cloned_at_destination = 210;

constexpr s64 pfs_read_file_size_target   = 65536;
constexpr s64 pfs_read_target             = 10656;
constexpr s64 pfs_getdirentries_target    = 10656;
constexpr s64 normal_read_target          = 8704;
constexpr s64 normal_getdirentries_target = 8704;

constexpr char DEFAULT_CHR = 'A';
constexpr u8   DEFAULT_8   = 0xAA;
constexpr u8   DEFAULT_16  = 0xAAAA;
constexpr u8   DEFAULT_32  = 0xAAAAAAAA;
constexpr u8   DEFAULT_64  = 0xAAAAAAAAAAAAAAAA;

#include "read_variants/variants.h"

#endif // FS_TEST_CONST_H