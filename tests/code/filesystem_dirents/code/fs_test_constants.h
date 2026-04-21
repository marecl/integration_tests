#ifndef FS_TEST_CONST_H
#define FS_TEST_CONST_H

#pragma once

#define FUZZ_SAMPLES 100

constexpr u64 pfs_read_file_size_target   = 65536;
constexpr u64 pfs_read_target             = 10656;
constexpr u64 pfs_getdirentries_target    = 10656;
constexpr u64 normal_read_target          = 8704;
constexpr u64 normal_getdirentries_target = 8704;

#include "read_variants/variants.h"

#endif // FS_TEST_CONST_H