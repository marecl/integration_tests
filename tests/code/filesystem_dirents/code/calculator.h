#ifndef CALCULATOR_H
#define CALCULATOR_H

#pragma once

#include "fs_test.h"

void calculate_pfs_read(OrbisInternals::DirentCombination* spec, s64 size, s64 offset, u64 count);
void calculate_normal_read(OrbisInternals::DirentCombination* spec, s64 size, s64 offset, u64 count);

void calculate_pfs_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, u64 count);
void calculate_normal_getdirentries(OrbisInternals::DirentCombination* spec, s64 size, s64 offset, u64 count);

s64 calculate_lseek(s64 directory_size, s64 file_offset, s64 offset, int whence, int* errno_ret = nullptr);

#endif // CALCULATOR_H