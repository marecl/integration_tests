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

// returns positive offset (how much to back off) or -1 if not found
s64 nearest_dirent(const char* buffer, s64 size, s64 offset) {
  // check test data for comments and how far it should look for dirents
  s64 max_backoff = offset >= 272 ? offset - 272 : 0;
  s64 max_front   = offset + 272 > size ? size - 24 : offset + 272;
  // min dirent length is 24, so if we can't find it 24 bytes before end then we're not gonan find it at all

  for (s64 new_offset = offset; new_offset >= max_backoff; new_offset -= 8) {
    auto status = validate_pfs_getdirentries_dirent(reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + new_offset));
    if (status == 1) {
      LogError("Found a match backwards at", new_offset);
      return new_offset;
    }
  }

  for (s64 new_offset = offset; new_offset <= max_front; new_offset += 8) {
    auto status = validate_pfs_getdirentries_dirent(reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + new_offset));
    if (status == 1) {
      LogError("Found a match forward at", new_offset);
      return new_offset;
    }
  }

  LogError("No match");
  return -1;
}

void calculate_pfs_getdirentries(OrbisInternals::DirentCombinationGetdirentries* spec, const char* buffer, s64 size, s64 offset, s64 count) {
  // down-aligned apparent end, last crossed sector

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
    int tmp               = ORBIS_KERNEL_ERROR_EINVAL;
    spec->expected_result = s64(tmp);
    spec->expected_errno  = EINVAL;
    return;
  }

  if (count > (size - offset)) return;

  if (offset >= size) {
    offset                      = directory_size;
    spec->expected_end_position = offset;
    spec->expected_result       = 0;
    return;
  }

  auto newidx = nearest_dirent(buffer, size, offset);
  if (newidx < 0) LogError("Can't back off to nearest valid dirent:", newidx);
  LogError("True starting offset is at", newidx);
  offset = newidx;

  s64 bytes_written   = 0;
  s64 buffer_position = offset;

  while (buffer_position < size) {
    const OrbisInternals::FolderDirent* pfs_dirent = reinterpret_cast<const OrbisInternals::FolderDirent*>(buffer + buffer_position);

    if ((bytes_written + pfs_dirent->d_reclen) > count) break;
    if ((bytes_written + pfs_dirent->d_reclen) > apparent_end_down) break;
    if (pfs_dirent->d_reclen == 0) break;

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