#pragma once

#include "fs_test.h"

void calculate_pfs_read(OrbisInternals::DirentCombinationRead* spec, const void* buffer, s64 size, s64 offset, u64 count);
void calculate_normal_read(OrbisInternals::DirentCombinationRead* spec, const void* buffer, s64 size, s64 offset, u64 count);

void calculate_pfs_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const void* buffer, s64 size, s64 offset, u64 count);
void calculate_normal_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const void* buffer, s64 size, s64 offset, u64 count);