#include "fs_test.h"

#include <cstring>
#include <filesystem>
#include <orbis/libkernel.h>
#include <vector>

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

// -1 = equal, anything positive - idx of first differing element
// does not adhere to memcmp spec, i want differing indexes, not values!
s64 imemcmp(const void* object, const void* reflection, s64 bytes) {
  u32        longs            = (bytes & ~0x7) >> 3; // bytes to longs (divide by 8 basically)
  const u64* object_ptr64     = static_cast<const u64*>(object);
  const u64* reflection_ptr64 = static_cast<const u64*>(reflection);
  const u8*  object_ptr8      = static_cast<const u8*>(object);
  const u8*  reflection_ptr8  = static_cast<const u8*>(reflection);
  u64        idx              = 0;

  for (idx = 0; idx < longs; idx++) {
    // target is 64bit, so one step is 8 bytes
    if (*(object_ptr64 + idx) == *(reflection_ptr64 + idx)) continue;
    // if different, skip the rest to compare shorts
    break;
  }

  // longlong->byte conversion
  // worst case scenario 8 iterations
  for (idx <<= 3; idx < bytes; idx++) {
    if (*(object_ptr8 + idx) != *(reflection_ptr8 + idx)) return idx;
  }

  return -1;
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

#define VNG_NL(x)     (x > 0)                                  // single byte, just not a 0
#define VNG_T(x)      (x >= 0 && x < 15)                       // TODO: check
#define VNG_RL(x)     (x >= 12 && x <= 264 && (x & 0x03) == 0) // min/max possible reclen, aligned to 4
#define VNG_NLE(x, y) (strnlen(x, 255) == y)                   // duuh

s64 validate_normal_getdirentries(const void* data, const s64 bytes) {
  if (bytes < 0) return bytes;

  const u8* data_ptr     = static_cast<const u8*>(data);
  s64       total_size   = 0;
  u32       current_size = 0;

  while (total_size < bytes) { // this element is in bounds
    const oi::FolderDirent* dirent = reinterpret_cast<const oi::FolderDirent*>(data_ptr + total_size);
    if (dirent->d_reclen == 0) {
      LogError("error: reclen = ", dirent->d_reclen);
      break;
    }; // likely went OOB
    if (!VNG_NL(dirent->d_namlen)) {
      LogError("error: namlen = ", dirent->d_namlen);
      break;
    };
    if (!VNG_T(dirent->d_type)) {
      LogError("error: type = ", dirent->d_type);
      break;
    };
    if (!VNG_RL(dirent->d_reclen)) {
      LogError("error: reclen = ", dirent->d_reclen);
      break;
    };
    if (!VNG_NLE(dirent->d_name, dirent->d_namlen)) {
      LogError("error: strlen = ", strlen(dirent->d_name));
      break;
    };
    total_size += dirent->d_reclen;
  }
  return total_size;
}

#define VPG_NL(x)     (x > 0 && x < 256)                         // string, so that's obvious (255+null)
#define VPG_T(x)      (x >= 0 && x < 15)                         // types cap at 15 i think
#define VPG_RL(x)     (x >= 24 && x <= 272 && ((x & 0x07) == 0)) // min/max possible reclen, aligned to 8
#define VPG_NLE(x, y) (strnlen(x, 255) == y)                     // duuh

s64 validate_pfs_getdirentries(const void* data, const s64 bytes) {
  if (bytes < 0) return bytes;

  const u8* data_ptr     = static_cast<const u8*>(data);
  s64       total_size   = 0;
  u32       current_size = 0;

  while (total_size < bytes) { // this element is in bounds
    const oi::PfsDirent* dirent = reinterpret_cast<const oi::PfsDirent*>(data_ptr + total_size);
    if (dirent->d_reclen == 0) {
      LogError("error: reclen = ", dirent->d_reclen);
      break;
    }; // likely went OOB
    if (!VPG_NL(dirent->d_namlen)) {
      LogError("error: namlen = ", dirent->d_namlen);
      break;
    };
    if (!VPG_T(dirent->d_type)) {
      LogError("error: type = ", dirent->d_type);
      break;
    };
    if (!VPG_RL(dirent->d_reclen)) {
      LogError("error: reclen = ", dirent->d_reclen);
      break;
    };
    if (!VPG_NLE(dirent->d_name, dirent->d_namlen)) {
      LogError("error: strlen = ", strlen(dirent->d_name));
      break;
    };
    total_size += dirent->d_reclen;
  }
  return total_size;
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

void RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
}
