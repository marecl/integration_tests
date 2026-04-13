#include "calculator.h"

/**
 * Do not update those routines!
 * Those should simulate how PS4 handles maths
 */

void calculate_pfs_read(OrbisInternals::DirentCombinationRead* spec, const void* buffer, s64 size, s64 offset, s64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - offset, static_cast<s64>(count));
  spec->expected_end_position = offset + spec->expected_result;
}

void calculate_normal_read(OrbisInternals::DirentCombinationRead* spec, const void* buffer, s64 size, s64 offset, s64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - offset, static_cast<s64>(count));
  spec->expected_end_position = offset + spec->expected_result;
}

#define ALUP(x, y) ((x & (~(y - 1))) + y)
#define ALDN(x, y) ((x & (~(y - 1))))

void calculate_pfs_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const void* buffer, s64 size, s64 offset, s64 count) {
  // down-aligned apparent end, last crossed sector
  spec->read_offset           = offset;
  spec->read_size             = count;
  spec->expected_basep        = offset;
  spec->expected_end_position = 0;
  spec->expected_result       = 0;
  spec->expected_errno        = 0;

  s64 apparent_end        = offset + count;
  s64 apparent_end_down   = ALDN(apparent_end, 512);
  s64 apparent_end_up     = ALUP(apparent_end, 512);
  s64 file_offset_aligned = ALDN(offset, 512);
  s64 directory_size      = ALUP(size, 0x10000);

  s64 file_offset_up = ALUP(offset, 512);

  if (apparent_end_up == file_offset_up) {
    int tmp               = ORBIS_KERNEL_ERROR_EINVAL;
    spec->expected_result = s64(tmp);
    spec->expected_errno  = EINVAL;
    return;
  }
  if (count < (file_offset_up - offset)) return;

  apparent_end_down   = std::min(size - offset, apparent_end_down);
  apparent_end_down   = std::min(count, apparent_end_down);
  u64 bytes_written   = 0;
  u64 buffer_position = offset;

  // same as others, we just don't need a variable
  if (offset >= size) {
    offset                      = directory_size;
    spec->expected_end_position = offset;
  } else {
    while ((bytes_written < apparent_end_down) && (buffer_position < size)) {
      const OrbisInternals::PfsDirent* pfs_dirent = reinterpret_cast<const OrbisInternals::PfsDirent*>(static_cast<const char*>(buffer) + buffer_position);
      // bad, incomplete or OOB entry
      if (pfs_dirent->d_reclen == 0) {
        LogError("0reclen");
        break;
      }

      if ((bytes_written + pfs_dirent->d_reclen) > count)
      // dirents are aligned to the last full one
      {
        LogError("oob");
        break;
      }

      // align to 65536 when dir size is bigger?
      // if (Common::AlignUp(buffer_position, count) !=
      //     Common::AlignUp(buffer_position + pfs_dirent->d_reclen, count))
      //     break;

      // reclen for both is the same despite difference in var sizes, extra 0s are padded after
      // the name
      bytes_written += pfs_dirent->d_reclen;
      buffer_position += pfs_dirent->d_reclen;
      LogError(bytes_written, "\t", buffer_position);
    }
  }

  spec->expected_result       = bytes_written;
  spec->expected_end_position = (buffer_position >= size) ? directory_size : static_cast<s64>(offset + bytes_written);
}

// void calculate_normal_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const void* buffer, s64 size, s64 offset, s64 count);