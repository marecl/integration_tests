#include "fs_test.h"

#include <cstring>
#include <filesystem>
#include <orbis/libkernel.h>
#include <vector>

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

s64 get_fuzz() {
  return ((s64(rand()) << 32) | rand()) >> (rand() % 64);
}

std::string val_or_err(s64 value) {
  switch (value) {
    default: return std::to_string(value);
    case einval_int: return "EINVAL";
    case enotty_int: return "ENOTTY";
    case enxio_int: return "ENXIO";
    case eoverflow_int: return "EOVERFLOW";
  }
  return "WHATTHEFUCK";
}

// 1 on equal, <=0 on diff idx
s64 compare_data_dump(const fuck& master, const fuck& test, s64 tbr, s64 offset) {
  s64 buffer_size = master.size();

  if (buffer_size <= 0) return 1;
  if (tbr <= 0) return 1;
  if (offset < 0) return 1;
  if (offset >= buffer_size) return 1;
  if (offset + tbr > buffer_size) {

    LogError("Partial comparsion possible:", buffer_size - offset, "/", tbr, "can be compared");
    tbr = buffer_size - offset;
  }

  if (s64 idx = imemcmp(master.data() + offset, test.data(), tbr); idx <= 0) {
    // differing idx is negative
    return idx;
  }
  return 1;
}

// 1 = equal, <=0 = diff idx
// does not adhere to memcmp spec, i want differing indexes, not values!
s64 imemcmp(const void* object, const void* reflection, s64 bytes) {
  if (bytes <= 0) return 1;

  u32        longs            = (bytes & ~0x7) >> 3; // bytes to longs (divide by 8 basically)
  const u64* object_ptr64     = static_cast<const u64*>(object);
  const u64* reflection_ptr64 = static_cast<const u64*>(reflection);
  const u8*  object_ptr8      = static_cast<const u8*>(object);
  const u8*  reflection_ptr8  = static_cast<const u8*>(reflection);
  s64        idx              = 0;

  for (idx = 0; idx < longs; idx++) {
    // target is 64bit, so one step is 8 bytes
    if (*(object_ptr64 + idx) == *(reflection_ptr64 + idx)) continue;
    // if different, skip the rest to compare shorts
    break;
  }

  // longlong->byte conversion
  // worst case scenario 8 iterations
  for (idx <<= 3; idx < bytes; idx++) {
    if (*(object_ptr8 + idx) == *(reflection_ptr8 + idx)) continue;
    return -idx;
  }

  return 1;
}

s64 fillcheck(const void* data, const u8 value, const u64 bytes) {
  u64 longval = 0x0101010101010101 * value; // fills entire u64 with one value
  u32 longs   = (bytes & ~0x7) >> 3;

  const u64* data_ptr64 = static_cast<const u64*>(data);
  const u8*  data_ptr8  = static_cast<const u8*>(data);
  u64        idx        = 0;

  for (idx = 0; idx < longs; idx++) {
    if (longval != *(data_ptr64 + idx)) break;
  }

  for (idx <<= 3; idx < bytes; idx++) {
    if (value != *(data_ptr8 + idx)) return idx;
  }

  return -1;
};

s64 validate_normal_dirent(const oi::FolderDirent* dirent) {
  // N/A it'd need offset to calculate reclen correctly
  // i think this test is duplicated somewhere here too
  // auto _reclen = 8 + dirent->d_namlen + 1;
  // _reclen      = ISAL(_reclen, 8) ? _reclen : ALUP(_reclen, 8);
  // if (_reclen != dirent->d_reclen) return -10;

  // best case scenario tbh
  if (dirent->d_reclen & 0x03) return -10;
  if (dirent->d_fileno == 0) return -11;

  // these don't fail so often
  if (dirent->d_namlen == 0) return -12;
  if (dirent->d_type == 0) return -13;
  if (dirent->d_reclen == 0) return -14;
  if (dirent->d_reclen < 12 || dirent->d_reclen > 512) return -16;
  if (dirent->d_type > 15) return -17;
  if (strnlen(dirent->d_name, 255) != dirent->d_namlen) return -18;

  return 1;
}

/**
 * reclen is basically a checksum
 * i've never seen any other than -11, but i've attached others for verbosity
 */

// pfs getdirentries returns normal direntries
s64 validate_pfs_read_dirent(const oi::PfsDirent* dirent) {
  auto _reclen = 16 + dirent->d_namlen + 1;
  _reclen      = ISAL(_reclen, 8) ? _reclen : ALUP(_reclen, 8);
  if (_reclen != dirent->d_reclen) return -10;
  if (dirent->d_fileno == 0) return -11;

  // these don't fail so often
  if (dirent->d_namlen == 0) return -12;
  if (dirent->d_type == 0) return -13;
  if (dirent->d_reclen == 0) return -14;
  if ((dirent->d_reclen & 0x07) != 0) return -15;
  if (dirent->d_reclen < 24 || dirent->d_reclen > 272) return -16;
  if (dirent->d_type > 15) return -17;
  if (strnlen(dirent->d_name, 255) != dirent->d_namlen) return -18;

  return 1;
}

// pfs getdirentries returns normal direntries
s64 validate_pfs_getdirentries_dirent(const oi::FolderDirent* dirent) {
  auto _reclen = 16 + dirent->d_namlen + 1;
  _reclen      = ISAL(_reclen, 8) ? _reclen : ALUP(_reclen, 8);
  if (_reclen != dirent->d_reclen) return -10;
  if (dirent->d_fileno == 0) return -11;

  // these don't fail so often
  if (dirent->d_namlen == 0) return -12;
  if (dirent->d_type == 0) return -13;
  if (dirent->d_reclen == 0) return -14;
  if ((dirent->d_reclen & 0x07) != 0) return -15;
  if (dirent->d_reclen < 24 || dirent->d_reclen > 272) return -16;
  if (dirent->d_type > 15) return -17;
  if (strnlen(dirent->d_name, 255) != dirent->d_namlen) return -18;

  return 1;
}

/**
 * I suspect that pfs dirents are u32 but need to be mask
 * maybe Orbis uses normal types, but on disk each one is u32 instead of u32,u16,u8,u8
 * so we can clear lower bytes and check for anything above!
 */

// pfs getdirentries returns normal direntries
s64 validate_pfs_getdirentries_experimental(const oi::PfsDirent* dirent) {
  if (dirent->d_type & ~s32(0xFF)) return -12;
  if (dirent->d_namlen & ~s32(0xFF)) return -13;
  if (dirent->d_reclen & ~s32(0xFFFF)) return -14;

  if (ALUP(16 + dirent->d_namlen + 1, 8) != dirent->d_reclen) return -15;
  if ((dirent->d_reclen < 24) || (dirent->d_reclen > 272)) return -16;
  if (dirent->d_type > 15) return -17;
  if (dirent->d_namlen == 0) return -18;
  if (strnlen(dirent->d_name, 255) != dirent->d_namlen) return -19;
  return 0;
}

s64 validate_normal_getdirentries(const char* data, const s64 bytes) {
  if (bytes < 0) return bytes;
  if (data == nullptr) return -1;

  s64 offset       = 0;
  u32 current_size = 0;

  while (offset < bytes) { // this element is in bounds
    const oi::FolderDirent* dirent = reinterpret_cast<const oi::FolderDirent*>(data + offset);
    auto                    vstat  = validate_normal_dirent(dirent);
    if (vstat < 0) {
      LogError("VSTAT", "=", vstat);
      return vstat;
    }

    s64 next_alignment = ALUP(offset, 512);
    if ((offset + dirent->d_reclen) > next_alignment) {
      LogError("Dirent not aligned to 512 byte sector at", offset, "leaking", offset + dirent->d_reclen - next_alignment, "bytes");
      return -2;
    }
    offset += dirent->d_reclen;
  }
  return offset;
}

s64 validate_pfs_read(const char* data, const s64 bytes) {
  if (bytes <= 0) return 0;

  s64 offset = 0;

  while (offset < bytes) { // this element is in bounds
    const oi::PfsDirent* dirent = reinterpret_cast<const oi::PfsDirent*>(data + offset);
    if (auto ret = validate_pfs_read_dirent(dirent); ret < 0) break;
    if (offset + dirent->d_reclen > bytes) break;
    offset += dirent->d_reclen;
  }
  return offset;
}

s64 validate_pfs_getdirentries(const char* data, const s64 bytes) {
  if (bytes < 0) return bytes;

  s64 offset = 0;

  while (offset < bytes) { // this element is in bounds
    const oi::FolderDirent* dirent = reinterpret_cast<const oi::FolderDirent*>(data + offset);
    if (auto ret = validate_pfs_getdirentries_dirent(dirent); ret < 0) break;
    if (offset + dirent->d_reclen > bytes) break;
    offset += dirent->d_reclen;
  }
  return offset;
}

bool is_directory_relatives(const char* data) {
  const u32 dotfinder = *reinterpret_cast<const u32*>(data);
  return ((dotfinder & 0x0000FFFF) == 0x002e) || ((dotfinder & 0x00FFFFFF) == 0x002e2e);
}

int32_t touch(const char* path) {
  return sceKernelClose(sceKernelOpen(path, O_CREAT | O_WRONLY | O_TRUNC, 0777));
}

int32_t touch(const std::string& path) {
  return sceKernelClose(sceKernelOpen(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777));
}

void Obliterate(const char* path) {
  Log("<< rm -rf [", path, "] >>");
  std::error_code ec {};

  std::vector<std::string> entries;
  for (auto& p: fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied, ec))
    entries.push_back(p.path().string());

  for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
    if (ec) {
      LogError("Exception: [", ec.value(), "] :", ec.message());
      ec.clear();
      continue;
    }

    const char* pp = it->c_str();

    // see what sticks
    errno = 0;
    if (0 == sceKernelUnlink(pp)) continue;
    errno = 0;
    if (0 == sceKernelRmdir(pp)) continue;
    LogError("Cannot remove [", pp, "] ( errno =", errno, ")");
  }
  if (0 != sceKernelRmdir(path) && errno == ENOENT) LogError("Cannot remove [", path, "] ( errno =", errno, ")");

  LogSuccess(">> rm -rf [", path, "] <<");
  return;
}

s64 get_stat_size(const char* path) {
  OrbisKernelStat st {};
  if (auto tmp = sceKernelStat(path, &st); tmp != 0) return tmp;
  return st.st_size;
}

void RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
}
