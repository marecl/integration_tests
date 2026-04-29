#ifndef FS_TEST_H
#define FS_TEST_H

#pragma once

#include "log.h"

#include <orbis/libkernel.h>

#define ALUP(x, y) (((x) & (~((y) - 1))) + (y))
#define ALDN(x, y) (((x) & (~((y) - 1))))
#define ISAL(x, y) (bool(((x) & ((y) - 1)) == 0))

#define UNSIGNED_INT_EQUALS(expected, actual)            UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, NULLPTR, __FILE__, __LINE__)
#define UNSIGNED_INT_EQUALS_TEXT(expected, actual, text) UNSIGNED_LONGS_EQUAL_LOCATION((uint32_t)expected, (uint32_t)actual, text, __FILE__, __LINE__)
constexpr int einval_int    = ORBIS_KERNEL_ERROR_EINVAL;
constexpr int enotty_int    = ORBIS_KERNEL_ERROR_ENOTTY;
constexpr int enxio_int     = ORBIS_KERNEL_ERROR_ENXIO;
constexpr int eoverflow_int = ORBIS_KERNEL_ERROR_EOVERFLOW;

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

namespace OrbisInternals {

typedef struct {
  u64 size;
  s64 offset;
} spec_t;

typedef struct {
  s64 offset;
  int whence;
} offset_spec_t;

typedef struct {
  s64 read_size;   // user-defined
  s64 read_offset; // user-defined
  // results, calculated:
  s64 expected_lseek;        // lseek result
  s64 expected_basep;        // read start position
  s64 meta_dirent_start;     // offset relative to offset of the first dirent (PFS getdirentries only)
  s64 expected_result;       // return from read/getdirentries
  s64 expected_end_position; // file pointer
  int expected_errno;
} DirentCombination;

typedef struct {
  s32  d_fileno;
  s32  d_type;
  s32  d_namlen;
  s32  d_reclen;
  char d_name[256];
} PfsDirent;

typedef struct {
  u32  d_fileno;
  u16  d_reclen;
  u8   d_type;
  u8   d_namlen;
  char d_name[256];
} FolderDirent;
} // namespace OrbisInternals

s64 get_fuzz();

bool    PrepareTests();
void    RegenerateDir(const char* path);
void    Obliterate(const char* path);
int32_t touch(const char* path);
int32_t touch(const std::string& path);
bool    is_directory_relatives(const char* data);
s64     imemcmp(const void* object, const void* reflection, s64 bytes);
s64     fillcheck(const void* data, const u8 value, const u64 bytes);

std::string val_or_err(s64 value);
s64         compare_data_dump(const void* master, const void* test, s64 buffer_size, s64 tbr, s64 offset);

s64 validate_normal_dirent(const OrbisInternals::FolderDirent* dirent);
s64 validate_pfs_read_dirent(const OrbisInternals::PfsDirent* dirent);
s64 validate_pfs_getdirentries_dirent(const OrbisInternals::FolderDirent* dirent);
s64 validate_pfs_getdirentries_experimental(const OrbisInternals::PfsDirent* dirent);

s64 validate_normal_getdirentries(const char* data, const s64 bytes);
s64 validate_pfs_read(const char* data, const s64 bytes);
s64 validate_pfs_getdirentries(const char* data, const s64 bytes);

#endif // FS_TEST_H
