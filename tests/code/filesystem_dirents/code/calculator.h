#ifndef CALCULATOR_H
#define CALCULATOR_H

#pragma once

#include "fs_test.h"

void calculate_pfs_read(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count);
void calculate_normal_read(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count);

void calculate_pfs_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count);
void calculate_normal_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count);

#endif // CALCULATOR_H