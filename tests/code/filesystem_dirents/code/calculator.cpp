#include "calculator.h"

#include "log.h"

/**
 * Do not update those routines!
 * Those should simulate how PS4 handles maths
 */

void calculate_pfs_read(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, u64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_lseek        = offset;
  spec->expected_basep        = -1;
  spec->meta_dirent_start     = 0; // unused, always equal to lseek-calculated offset
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  spec->expected_lseek    = calculate_lseek(size, 0, offset, 0, &spec->expected_errno);
  s64 new_offset          = spec->expected_lseek >= 0 ? spec->expected_lseek : 0;
  spec->expected_basep    = new_offset;
  spec->meta_dirent_start = new_offset;

  // if (count > 0x7ffff000) {
  //   spec->expected_basep        = new_offset;
  //   spec->expected_result       = einval_int;
  //   spec->expected_end_position = (new_offset >= 0) ? new_offset : 0;
  //   spec->expected_errno        = EINVAL;
  //   return;
  // }

  spec->expected_errno = 0;

  if (new_offset > size) return;

  spec->expected_basep        = new_offset;
  spec->expected_result       = std::min<s64>(size - spec->expected_basep, static_cast<s64>(count));
  spec->expected_end_position = spec->expected_basep + spec->expected_result;
}

void calculate_normal_read(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, u64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_lseek        = offset;
  spec->expected_basep        = -1;
  spec->meta_dirent_start     = 0; // unused, always equal to lseek-calculated offset
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  spec->expected_lseek    = calculate_lseek(size, 0, offset, 0, &spec->expected_errno);
  s64 new_offset          = spec->expected_lseek >= 0 ? spec->expected_lseek : 0;
  spec->expected_basep    = new_offset;
  spec->meta_dirent_start = new_offset;

  // if (count > 0x7ffff000) {
  //   spec->expected_basep        = new_offset;
  //   spec->expected_result       = einval_int;
  //   spec->expected_end_position = (new_offset >= 0) ? new_offset : 0;
  //   spec->expected_errno        = EINVAL;
  //   return;
  // }

  if (offset > size) {
    spec->expected_lseek = new_offset;
    return;
  }

  spec->expected_basep        = new_offset;
  spec->expected_result       = std::min<s64>(size - spec->expected_basep, static_cast<s64>(count));
  spec->expected_end_position = new_offset + spec->expected_result;
}

// -1 on not found
// this returns relative offset to starting position
s64 nearest_dirent(const char* buffer, s64 size, s64 offset) {
  // max size is 272, last 23 bytes are never starting a dirent

  s64 offset_adj  = ISAL(offset, 8) ? offset : ALUP(offset, 8);
  s64 max_advance = std::min(size - offset_adj, s64(272));
  if (max_advance < 24) return -2;

  s64 status {};

  for (s64 out_offset = offset_adj; out_offset <= offset_adj + max_advance; out_offset += 8) {
    const OrbisInternals::FolderDirent* tested_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + out_offset);
    status                                            = validate_pfs_getdirentries_dirent(tested_dirent);

    if (status < 0) continue;

    return out_offset - offset;
  }

  return status;
}

void calculate_pfs_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, u64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_basep        = -1;
  spec->meta_dirent_start     = 0; // unused, always equal to lseek-calculated offset
  spec->expected_result       = 0;
  spec->expected_end_position = 0;
  spec->expected_errno        = 0;

  spec->expected_lseek    = calculate_lseek(size, 0, offset, 0, &spec->expected_errno);
  s64 new_offset          = spec->expected_lseek >= 0 ? spec->expected_lseek : 0;
  spec->meta_dirent_start = new_offset;

  s64 apparent_end      = new_offset + count;
  s64 apparent_end_down = ALDN(apparent_end, 512);
  s64 file_offset_down  = ALDN(new_offset, 512);
  s64 directory_size    = ISAL(size, 0x10000) ? size : ALUP(size, 0x10000);

  // within the same sector, no 512b alignment inbetween
  if (apparent_end_down <= file_offset_down) {
    spec->expected_basep        = -1;
    spec->expected_result       = s64(einval_int);
    spec->expected_end_position = new_offset;
    spec->expected_errno        = EINVAL;
    // LogError("Dent align apend-off");
    return;
  }

  if (new_offset >= directory_size) {
    spec->expected_basep        = new_offset;
    spec->expected_result       = 0;
    spec->expected_end_position = new_offset;
    spec->expected_errno        = 0;
    // LogError("Dent oob");
    return;
  }

  if (new_offset >= size) {
    spec->expected_basep        = new_offset;
    spec->expected_result       = 0;
    spec->expected_end_position = directory_size;
    spec->expected_errno        = 0;
    return;
  }

  // we can now assume that offset is always smaller than size
  s64 dirent_offset = nearest_dirent(buffer, size, new_offset);

  if (dirent_offset < 0) {
    // highly unlikely but you never know
    spec->expected_basep        = new_offset;
    spec->expected_result       = 0;
    spec->expected_end_position = directory_size;
    spec->expected_errno        = 0;
    return;
  };

  s64 bytes_written   = 0;
  s64 buffer_position = new_offset;
  s64 allowed_count   = std::min(apparent_end_down - new_offset, s64(count));
  allowed_count       = std::min(allowed_count, size - new_offset);

  if (allowed_count <= 0) {
    spec->expected_basep        = new_offset;
    spec->expected_result       = 0;
    spec->expected_end_position = new_offset;
    spec->expected_errno        = 0;
    return;
  }

  while (bytes_written < allowed_count) {
    const OrbisInternals::FolderDirent* pfs_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + buffer_position + dirent_offset);
    if ((bytes_written + pfs_dirent->d_reclen) > allowed_count) {
      break;
    }
    if (pfs_dirent->d_reclen == 0) {
      break;
    }

    // reclen for both is the same despite difference in var sizes, extra 0s are padded after
    // the name
    bytes_written += pfs_dirent->d_reclen;
    buffer_position += pfs_dirent->d_reclen;
  }

  spec->expected_basep        = new_offset;
  spec->meta_dirent_start     = new_offset + dirent_offset;
  spec->expected_result       = bytes_written;
  spec->expected_end_position = ((buffer_position + dirent_offset) >= size) ? directory_size : static_cast<s64>(new_offset + bytes_written);
  spec->expected_errno        = 0;
}

void calculate_normal_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, u64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_basep        = -1; // DON'T TOUCH. getdirentries sets it after non-0 amount of data to read
  spec->meta_dirent_start     = 0;  // unused, always equal to lseek-calculated offset
  spec->expected_result       = 0;
  spec->expected_end_position = 0;
  spec->expected_errno        = 0;

  spec->expected_lseek        = calculate_lseek(size, 0, offset, 0, &spec->expected_errno);
  s64 new_offset              = spec->expected_lseek >= 0 ? spec->expected_lseek : 0;
  spec->meta_dirent_start     = new_offset;
  spec->expected_end_position = new_offset;

  s64 apparent_end      = new_offset + count;
  s64 apparent_end_down = ALDN(apparent_end, 512);
  s64 file_offset_down  = ALDN(new_offset, 512);

  // within the same sector, no 512b alignment inbetween
  if (apparent_end_down <= file_offset_down) {
    spec->expected_result = s64(einval_int);
    spec->expected_errno  = EINVAL;
    return;
  }

  spec->expected_basep = new_offset;

  if (new_offset >= size) {
    spec->expected_result = 0;
    spec->expected_errno  = 0;
    return;
  }

  // we can now assume that offset is always smaller than size
  s64 allowed_count           = std::min(u64(apparent_end_down - new_offset), count);
  allowed_count               = std::min(allowed_count, size - new_offset);
  spec->expected_result       = allowed_count;
  spec->expected_end_position = static_cast<s64>(spec->expected_basep + allowed_count);
  spec->expected_errno        = 0;
}

s64 calculate_lseek(s64 directory_size, s64 file_offset, s64 offset, int whence, int* errno_ret) {
  *errno_ret = 0;

  if (whence < 0 || whence > 4) {
    if (nullptr != errno_ret) *errno_ret = EINVAL;
    return einval_int;
  }
  if (whence == 3 || whence == 4) {
    if (nullptr != errno_ret) *errno_ret = ENOTTY;
    return enotty_int;
  }

  s64 target_offset {};
  s64 offset_modificator =            //((0 == whence)) * 0 +    // there are no other options really lol
      ((1 == whence) * file_offset) + //
      ((2 == whence) * directory_size);
  if (__builtin_add_overflow(offset, offset_modificator, &target_offset)) {
    if (nullptr != errno_ret) *errno_ret = EOVERFLOW;
    return eoverflow_int;
  }
  if (target_offset < 0) {
    if (nullptr != errno_ret) *errno_ret = EINVAL;
    return einval_int;
  }

  return target_offset;
}