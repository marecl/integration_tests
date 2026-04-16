#include "calculator.h"

/**
 * Do not update those routines!
 * Those should simulate how PS4 handles maths
 */

void calculate_pfs_read(OrbisInternals::DirentCombinationRead* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  static s64 previous_offset = 0;
  offset                     = offset == -1 ? previous_offset : offset;
  previous_offset            = offset;

  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - offset, static_cast<s64>(count));
  spec->expected_end_position = offset + spec->expected_result;
}

void calculate_normal_read(OrbisInternals::DirentCombinationRead* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  static s64 previous_offset = 0;
  offset                     = offset == -1 ? previous_offset : offset;
  previous_offset            = offset;

  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - offset, static_cast<s64>(count));
  spec->expected_end_position = offset + spec->expected_result;
}

// -1 on not found
s64 nearest_dirent(const char* buffer, s64 size, s64 offset) {
  // max size is 272, last 23 bytes are never starting a dirent

  s64 offset_adj  = ISAL(offset, 8) ? offset : ALUP(offset, 8);
  s64 max_advance = std::min(size - offset_adj, s64(272));
  if (max_advance < 24) return -2;

  s64 status {};

  for (s64 out_offset = offset_adj; out_offset <= offset_adj + max_advance; out_offset += 8) {
    const OrbisInternals::FolderDirent* tested_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + out_offset);
    status                                            = validate_pfs_getdirentries_dirent(tested_dirent);
    // LogError("Testing", out_offset, status);
    if (status < 0) continue;

    // LogError("Found a match forward at", out_offset);
    return out_offset - offset;
  }

  // LogError("No match");
  return status;
}

void calculate_pfs_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  // down-aligned apparent end, last crossed sector

  int        einval_int      = ORBIS_KERNEL_ERROR_EINVAL;
  static s64 previous_offset = 0;
  offset                     = offset == -1 ? previous_offset : offset;
  previous_offset            = offset;

  spec->read_offset           = offset;
  spec->read_size             = count;
  spec->expected_basep        = offset;
  spec->expected_end_position = 0;
  spec->expected_result       = 0;
  spec->expected_errno        = 0;

  s64 apparent_end      = offset + count;
  s64 apparent_end_down = ALDN(apparent_end, 512);
  s64 file_offset_down  = ALDN(offset, 512);
  s64 directory_size    = ALUP(size, 0x10000);

  // within the same sector or worse
  if (apparent_end_down <= file_offset_down) {
    spec->expected_result = s64(einval_int);
    spec->expected_errno  = EINVAL;
    return;
  }

  if (offset >= size) {
    spec->expected_end_position = directory_size;
    spec->expected_basep        = offset;
    spec->expected_result       = 0;
    // LogError("qweqweqwe");
    return;
  }

  if (count > (size - offset)) {
    spec->expected_end_position = directory_size;
    spec->expected_basep        = offset;
    spec->expected_result       = 0;
    // LogError("cvcvcc");
    return;
  }

  s64 dirent_offset = nearest_dirent(buffer, size, offset);

  if (dirent_offset < 0) {
    LogError("Can't seek to the next dirent:", dirent_offset);
    spec->expected_result = s64(einval_int);
    spec->expected_errno  = EINVAL;
    return;
  };
  LogError("True starting offset is at", dirent_offset);

  s64 bytes_written   = 0;
  s64 buffer_position = offset;

  while (buffer_position < apparent_end) {
    const OrbisInternals::FolderDirent* pfs_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + buffer_position + dirent_offset);

    if (pfs_dirent->d_reclen == 0) break;
    if ((bytes_written + dirent_offset + pfs_dirent->d_reclen) > count) break;
    // without dirent offset i think
    // it sometimes underreads data at this line
    //
    if ((buffer_position + dirent_offset + pfs_dirent->d_reclen) >= apparent_end_down) {
      LogError("XVFED");
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
  }

  spec->expected_result       = bytes_written;
  spec->expected_end_position = (buffer_position >= size) ? directory_size : static_cast<s64>(offset + bytes_written);
}

// void calculate_normal_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const char* buffer, s64 size, s64 offset, s64 count);