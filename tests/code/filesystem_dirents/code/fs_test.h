#ifndef FS_TEST_H
#define FS_TEST_H

#define UNSIGNED_INT_EQUALS(expected, actual)            UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, NULLPTR, __FILE__, __LINE__)
#define UNSIGNED_INT_EQUALS_TEXT(expected, actual, text) UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, text, __FILE__, __LINE__)

#include "log.h"

#include <orbis/libkernel.h>

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

namespace OrbisInternals {

typedef struct DirentCombinationRead {
  s64 read_size;
  s64 read_offset;
  s64 expected_result;
  u32 expected_errno;
} DirentCombinationRead;

typedef struct DirentCombinationGetdirentries {
  s64 read_size;
  s64 read_offset;
  s64 expected_basep;
  s64 expected_result;
  u32 expected_errno;
} DirentCombinationGetdirentries;

typedef struct PfsDirent {
  s32  d_fileno;
  s32  d_type;
  s32  d_namlen;
  s32  d_reclen;
  char d_name[256];
} PfsDirent;

typedef struct FolderDirent {
  u32  d_fileno;
  u16  d_reclen;
  u8   d_type;
  u8   d_namlen;
  char d_name[256];
} FolderDirent;
} // namespace OrbisInternals

bool    PrepareTests();
void    RegenerateDir(const char* path);
void    Obliterate(const char* path);
int32_t touch(const char* path);
int32_t touch(const std::string& path);

bool is_directory_relatives(const char* data);
s64  qmemcmp(const void* object, const void* reflection, s64 bytes);
bool fillcheck(const void* data, const u8 value, const u64 bytes);

s64 validate_pfs_getdirentries(const void* data, const s64 bytes);

#endif // FS_TEST_H
