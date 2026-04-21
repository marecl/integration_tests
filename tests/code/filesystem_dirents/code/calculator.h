#ifndef CALCULATOR_H
#define CALCULATOR_H

#pragma once

#include "fs_test.h"

void calculate_pfs_read(OrbisInternals::DirentCombinationRead* spec, const char* buffer, s64 size, s64 offset, s64 count);
void calculate_normal_read(OrbisInternals::DirentCombinationRead* spec, const char* buffer, s64 size, s64 offset, s64 count);

void calculate_pfs_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const char* buffer, s64 size, s64 offset, s64 count);
void calculate_normal_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const char* buffer, s64 size, s64 offset, s64 count);

#endif // CALCULATOR_H