#include "calculator.h"

/**
 * Do not update those routines!
 * Those should simulate how PS4 handles maths
 */

void calculate_pfs_read(OrbisInternals::DirentCombinationRead* spec, const void* buffer, s64 size, s64 offset, u64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = offset;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - offset, static_cast<s64>(count));
  spec->expected_end_position = offset + spec->expected_result;
}

void calculate_normal_read(OrbisInternals::DirentCombinationRead* spec, const void* buffer, s64 size, s64 offset, u64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = offset;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - offset, static_cast<s64>(count));
  spec->expected_end_position = offset + spec->expected_result;
}

// void calculate_pfs_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const void* buffer, s64 size, s64 offset, u64 count);
// void calculate_normal_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const void* buffer, s64 size, s64 offset, u64 count);