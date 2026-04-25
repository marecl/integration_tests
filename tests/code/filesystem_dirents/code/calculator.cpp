#include "calculator.h"

#include "log.h"

/**
 * Do not update those routines!
 * Those should simulate how PS4 handles maths
 */

void calculate_pfs_read(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_lseek        = offset;
  spec->expected_basep        = offset;
  spec->meta_dirent_start     = offset; // unused here
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  if (offset < 0) { // this comes from lseek
    spec->expected_lseek = einval_int;
    spec->expected_basep = 0;
    spec->expected_errno = EINVAL; // not checked
  }

  if (count < 0) {
    spec->expected_result       = einval_int;
    spec->expected_end_position = (offset >= 0) ? offset : 0;
    spec->expected_errno        = EINVAL;
    return;
  }

  spec->expected_errno = 0;

  if (offset > size) return;

  spec->expected_result       = std::min<s64>(size - spec->expected_basep, static_cast<s64>(count));
  spec->expected_end_position = spec->expected_basep + spec->expected_result;
}

void calculate_normal_read(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  spec->read_size             = count;
  spec->read_offset           = offset;
  spec->expected_lseek        = offset;
  spec->expected_basep        = offset;
  spec->meta_dirent_start     = offset; // unused here
  spec->expected_result       = 0;
  spec->expected_end_position = offset;
  spec->expected_errno        = 0;

  if (offset < 0) { // this comes from lseek
    spec->expected_lseek = einval_int;
    spec->expected_basep = 0;
    spec->expected_errno = EINVAL; // not checked
  }

  if (count < 0) {
    spec->expected_result       = einval_int;
    spec->expected_end_position = (offset >= 0) ? offset : 0;
    spec->expected_errno        = EINVAL;
    return;
  }

  spec->expected_errno = 0;

  if (offset > size) {
    spec->expected_lseek = offset;
    return;
  }

  spec->expected_result       = std::min<s64>(size - spec->expected_basep, static_cast<s64>(count));
  spec->expected_end_position = spec->expected_basep + spec->expected_result;
}

// -1 on not found
// this returns relative offset to starting position
s64 nearest_dirent(const char* buffer, s64 size, s64 offset) {
  // max size is 272, last 23 bytes are never starting a dirent

  s64 offset_adj  = ISAL(offset, 8) ? offset : ALUP(offset, 8);
  s64 max_advance = std::min(size - offset_adj, s64(272));
  if (max_advance < 24) return -2;

  s64 status {};

  // LogWarning("Starting search at", offset, "->", offset_adj);
  for (s64 out_offset = offset_adj; out_offset <= offset_adj + max_advance; out_offset += 8) {
    const OrbisInternals::FolderDirent* tested_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + out_offset);
    status                                            = validate_pfs_getdirentries_dirent(tested_dirent);
    // LogWarning(GetSt(Style::BOLD), "Testing", out_offset - offset, GetSt(Style::RESET), "=", GetSt(Style::FG_MAGENTA), status,
    //  to_hex_string(buffer + out_offset, 16, ""));

    if (status < 0) continue;

    // LogError("Found a match forward at", out_offset);
    return out_offset - offset;
  }

  // LogError("No match");
  return status;
}

void calculate_pfs_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  static s64 previous_basep {0};

  spec->read_size   = count;
  spec->read_offset = offset;
  // spec->expected_lseek        = offset;
  spec->expected_basep        = -1;
  spec->meta_dirent_start     = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = 0;
  spec->expected_errno        = 0;

  spec->expected_lseek = calculate_lseek(size, 0, offset, 0);
  if (spec->expected_lseek < 0) {
    spec->expected_errno = EINVAL;
  }
  spec->expected_end_position = spec->expected_lseek >= 0 ? spec->expected_lseek : previous_basep;

  if (count <= 0) {
    spec->expected_result = einval_int;
    spec->expected_errno  = EINVAL;
    return;
  }

  s64 apparent_end      = offset + count;
  s64 apparent_end_down = ALDN(apparent_end, 512);
  s64 file_offset_down  = ALDN(offset, 512);
  s64 directory_size    = ALUP(size, 0x10000);

  // within the same sector, no 512b alignment inbetween
  if (apparent_end_down <= file_offset_down) {
    spec->expected_result       = s64(einval_int);
    spec->expected_end_position = offset;
    spec->expected_errno        = EINVAL;
    return;
  }

  previous_basep       = spec->expected_basep;
  spec->expected_basep = spec->expected_lseek >= 0 ? spec->expected_lseek : previous_basep;

  if (offset >= size) {
    spec->expected_result       = 0;
    spec->expected_end_position = directory_size;
    spec->expected_errno        = 0;
    return;
  }
  // we can now assume that offset always includes some data

  s64 dirent_offset = nearest_dirent(buffer, size, offset);

  if (dirent_offset < 0) {
    // highly unlikely but you never know
    spec->expected_basep        = previous_basep;
    spec->expected_result       = s64(einval_int);
    spec->expected_end_position = offset;
    spec->expected_errno        = EINVAL;
    return;
  };
  // LogError("True starting offset is at", dirent_offset);

  s64 bytes_written   = 0;
  s64 buffer_position = offset;
  s64 allowed_count   = std::min(apparent_end_down - offset, count);

  while (bytes_written < allowed_count) {
    const OrbisInternals::FolderDirent* pfs_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + buffer_position + dirent_offset);
    if ((bytes_written + pfs_dirent->d_reclen) >= allowed_count) break;
    if (pfs_dirent->d_reclen == 0) break;

    // reclen for both is the same despite difference in var sizes, extra 0s are padded after
    // the name
    bytes_written += pfs_dirent->d_reclen;
    buffer_position += pfs_dirent->d_reclen;
  }

  spec->expected_errno        = 0;
  spec->meta_dirent_start     = offset + dirent_offset;
  spec->expected_result       = bytes_written;
  spec->expected_end_position = ((buffer_position + dirent_offset) >= size) ? directory_size : static_cast<s64>(offset + bytes_written);
  previous_basep              = spec->expected_lseek;
}

void calculate_normal_getdirentries(OrbisInternals::DirentCombination* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  static s64 previous_basep = 0;

  spec->read_size   = count;
  spec->read_offset = offset;
  // spec->expected_lseek        = 0;
  spec->expected_basep        = -1; // DON'T TOUCH. getdirentries sets it after non-0 amount of data to read
  spec->meta_dirent_start     = offset;
  spec->expected_result       = 0;
  spec->expected_end_position = 0;
  spec->expected_errno        = 0;

  spec->expected_lseek = calculate_lseek(size, 0, offset, 0);
  if (spec->expected_lseek < 0) {
    spec->expected_errno = EINVAL;
  }

  spec->expected_end_position = spec->expected_lseek >= 0 ? spec->expected_lseek : 0;

  if (count < 0) {
    spec->expected_result = einval_int;
    spec->expected_errno  = EINVAL;
    return;
  }

  s64 apparent_end      = offset + count;
  s64 apparent_end_down = ALDN(apparent_end, 512);

  // within the same sector, no 512b alignment inbetween
  if (apparent_end_down <= ALDN(offset, 512)) {
    spec->expected_result = s64(einval_int);
    // spec->expected_end_position = offset;
    spec->expected_errno = EINVAL;
    return;
  }

  previous_basep       = spec->expected_basep;
  spec->expected_basep = spec->expected_lseek >= 0 ? spec->expected_lseek : previous_basep;

  if (offset >= size) {
    // spec->expected_basep        = offset;
    spec->expected_result = 0;
    // spec->expected_end_position = offset;
    spec->expected_errno = 0;
    return;
  }
  // we can now assume that offset always includes some data
  s64 allowed_count           = std::min(apparent_end_down - offset, count);
  allowed_count               = std::min(allowed_count, size - offset);
  spec->expected_result       = allowed_count;
  spec->expected_end_position = static_cast<s64>(spec->expected_basep + allowed_count);
}

s64 calculate_lseek(s64 directory_size, s64 file_offset, s64 offset, int whence) {
  // there's also whence 3,4 but we're not implementing that (yet?)
  // 3 and 4 return ENOTTY on some kind of error though
  if (whence < 0 || whence > 2) return einval_int;
  if ((directory_size + offset < 0)) return einval_int;

  s64 file_offset_new = ((0 == whence) * offset) + ((1 == whence) * (file_offset + offset)) + ((2 == whence) * (directory_size + offset));
  if (file_offset_new < 0) return einval_int;

  return file_offset_new;
}