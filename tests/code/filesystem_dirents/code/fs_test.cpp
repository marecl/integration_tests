#include "fs_test.h"

#include "orbis/UserService.h"

#include <CppUTest/TestHarness.h>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

std::vector<u32> read_sizes {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072};
std::vector<u16> read_offsets {0};

// std::vector<u32> read_sizes {// 8
//                              7, 8, 9,
//                              // 16
//                              15, 16, 17,
//                              // 32
//                              31, 32, 33,
//                              // 64
//                              63, 64, 65,
//                              // 128
//                              127, 128, 129,
//                              // 256
//                              255, 256, 257,
//                              // 512
//                              511, 512, 513,
//                              // 1024
//                              1023, 1024, 1025,
//                              // 2048
//                              2047, 2048, 2049,
//                              // 4096
//                              4095, 4096, 4097,
//                              // 65536
//                              65535, 65536, 65537,
//                              // cursed
//                              2137, 21, 37, 69, 420, 42, 123, 222, 666, 911, 112, 997,
//                              // something for zoomers
//                              67};
// std::vector<u16> read_offsets {0, 1, 5, 10, 21, 37, 127, 128, 129, 400, 500, 512, 768, 1024, 111, 666, 420, 1234, 96, 42};

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

s64  DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size);
s64  DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx);
void DumpDirectory(int fd, int buffer_size, s64 offset);
void DumpDirectoryPFS(int fd, int buffer_size, s64 offset);

void RunTests() {
  ///
  /// Setup dimping directory
  ///

  fs::path directory_source      = "/app0/assets/misc";
  fs::path clone_target_getdents = "/data/enderman/files/clone_getdents";
  fs::path clone_target_read     = "/data/enderman/files/clone_read";

  sceKernelMkdir(clone_target_read.c_str(), 0777);
  sceKernelMkdir(clone_target_getdents.c_str(), 0777);

  std::vector<std::string> entries_read {};
  std::vector<std::string> entries_getdirentries {};

  //

  Log("----------------------------------------");
  Log("Clone PFS (getdents) to /data/enderman/files");
  Log("----------------------------------------");
  // getdirentries GUARANTEES there are no dirents cut

  s64           read_start_position {0};
  constexpr int clone_buffer_size = 2048;
  char          clone_buffer[clone_buffer_size] {0};
  memset(clone_buffer, 0, clone_buffer_size);
  s64               tbr {0};
  s64               _idx = 0;
  u32               idx {0};
  oi::FolderDirent* dirent_normal {0};
  u16               _canary {0};

  int fd_src = sceKernelOpen(directory_source.c_str(), O_RDONLY | O_DIRECTORY, 0777);

  do {
    read_start_position = sceKernelLseek(fd_src, 0, 1);
    tbr                 = sceKernelGetdirentries(fd_src, clone_buffer, clone_buffer_size, &_idx);
    Log("Read:", tbr, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
    if (tbr <= 0) break;

    dirent_normal = reinterpret_cast<oi::FolderDirent*>(clone_buffer);
    for (idx = 0; (idx < tbr) && (dirent_normal->d_reclen > 0); idx += dirent_normal->d_reclen) {
      dirent_normal = reinterpret_cast<oi::FolderDirent*>(clone_buffer + idx);
      Log("start:", read_start_position, "+", idx, "fileno:", dirent_normal->d_fileno, "name:", dirent_normal->d_name,
          "type:", static_cast<u16>(dirent_normal->d_type), "namelen:", static_cast<u16>(dirent_normal->d_namlen), "reclen:", dirent_normal->d_reclen);
      entries_getdirentries.emplace_back(std::string(dirent_normal->d_name, dirent_normal->d_namlen));
    }
    Log("End position:", idx, "/", clone_buffer_size);
  } while (--_canary);

  sceKernelClose(fd_src);
  Log("Dump ended with status", tbr);

  //

  Log("----------------------------------------");
  Log("Clone PFS (read) to /data/enderman/files");
  Log("----------------------------------------");

  memset(clone_buffer, 0, clone_buffer_size);
  tbr = 0;
  idx = 0;
  oi::PfsDirent* dirent_pfs {0};
  _canary = 0;

  fd_src = sceKernelOpen(directory_source.c_str(), O_RDONLY | O_DIRECTORY, 0777);
  do {
    idx                 = 0;
    read_start_position = sceKernelLseek(fd_src, 0, 1);
    tbr                 = sceKernelRead(fd_src, clone_buffer, clone_buffer_size);
    Log("Read:", tbr, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
    if (tbr <= 0) break;

    dirent_pfs = reinterpret_cast<oi::PfsDirent*>(clone_buffer);
    // if less than 8 bytes is available, next dirent is 100% cut
    while (dirent_pfs->d_reclen > 0) {
      // get current dirent, see what we have
      Log("start:", read_start_position, "+", idx, "fileno:", dirent_pfs->d_fileno, "name:", dirent_pfs->d_name, "type:", dirent_pfs->d_type,
          "namelen:", static_cast<u16>(dirent_pfs->d_namlen), "reclen:", dirent_pfs->d_reclen);

      // more than 8 bytes available, but not the whole thing
      if ((idx + dirent_pfs->d_reclen) > tbr) {
        Log("Long dirent at", idx, "goes OOB");
        // idx not modified, just break
        break;
      }

      // on a good run, this increments correctly
      entries_read.emplace_back(std::string(dirent_pfs->d_name, dirent_pfs->d_namlen));
      idx += dirent_pfs->d_reclen;
      dirent_pfs = reinterpret_cast<oi::PfsDirent*>(clone_buffer + idx);

      // next dirent won't fit (24 bytes minimum)
      if ((tbr - idx) < 24) {
        Log("Dirent at", idx, "goes OOB");
        break;
      }
    }

    if ((idx != tbr) && (idx > 0)) sceKernelLseek(fd_src, read_start_position + idx, 0);
    Log("End position:", idx, "/", clone_buffer_size);
  } while (--_canary);

  sceKernelClose(fd_src);
  Log("Dump ended with status", tbr)

      Log("-------------");
  Log("Compare dumps");
  Log("-------------");

  std::sort(entries_getdirentries.begin(), entries_getdirentries.end());
  std::sort(entries_read.begin(), entries_read.end());

  if (entries_getdirentries.size() != entries_read.size()) {
    LogError("Sizes don't match. Getdirentries:", entries_getdirentries.size(), "read:", entries_read.size())
  }

  if (!std::equal(entries_getdirentries.begin(), entries_getdirentries.end(), entries_read.begin(), entries_read.end())) {
    LogError("Vectors aren't equal");
  }

  // Log("---------------------");
  // Log("Dump normal directory");
  // Log("---------------------");

  // int fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  // for (auto read_size: read_sizes) {
  //   for (auto read_offset: read_offsets) {
  //     DumpDirectory(fd, read_size, read_offset);
  //   }
  // }
  // sceKernelClose(fd);

  // Log("------------------");
  // Log("Dump PFS directory");
  // Log("------------------");
  // fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);
  // for (auto read_size: read_sizes) {
  //   for (auto read_offset: read_offsets) {
  //     DumpDirectory(fd, read_size, read_offset, true);
  //   }
  // }
  // sceKernelClose(fd);
}

s64 DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size) {
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelRead(dir_fd, buffer, size);
  // Log("Read got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1));

  if (tbr < 0) {
    LogError("Read finished with error:", tbr);
    return tbr;
  }

  if (s64 tbw = sceKernelWrite(dump_fd, buffer, tbr); tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes");
  return tbr;
}

s64 DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx) {
  // magic to determine how many trailing elements were cut
  memset(buffer, 0xAA, size);

  s64 tbr = sceKernelGetdirentries(dir_fd, buffer, size, idx);
  // Log("Dirent got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1), "idx =", *idx);

  if (tbr < 0) {
    LogError("Dirent finished with error:", tbr);
    return tbr;
  }

  if (s64 tbw = sceKernelWrite(dump_fd, buffer, tbr); tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes");
  return tbr;
}

void DumpDirectory(int fd, int buffer_size, s64 offset) {
  char* buffer = new char[buffer_size] {0};

  std::string file_basename = "dump_" + std::to_string(buffer_size) + '+' + std::to_string(offset);

  fs::path read_path       = "/data/enderman/dump_data_read/" + file_basename + ".bin";
  fs::path dirent_path     = "/data/enderman/dump_data_getdents/" + file_basename + ".bin";
  fs::path read_cue_path   = "/data/enderman/dump_data_read/" + file_basename + ".cue";
  fs::path dirent_cue_path = "/data/enderman/dump_data_getdents/" + file_basename + ".cue";

  LogTest("normal", "directory, fd =", fd, "buffer size =", buffer_size, "starting offset =", offset);

  s64 tbr         = 0;
  u16 max_loops   = 0; // 65536 iterations lmao
  int fd_read     = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int fd_read_cue = sceKernelOpen(read_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops) {
    tbr = DumpByRead(fd, fd_read, buffer, buffer_size);
    sceKernelWrite(fd_read_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
    if (tbr <= 0) break;
  }
  if (0 == max_loops) LogError("Aborted");
  sceKernelClose(fd_read);
  sceKernelClose(fd_read_cue);

  s64 idx           = 0;
  max_loops         = 0;
  int fd_dirent     = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int fd_dirent_cue = sceKernelOpen(dirent_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops) {
    tbr = DumpByDirent(fd, fd_dirent, buffer, buffer_size, &idx);
    sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
    sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&idx), sizeof(s64) / sizeof(u8));
    if (tbr <= 0) break;
  }
  if (0 == max_loops) LogError("Aborted");
  sceKernelClose(fd_dirent);
  sceKernelClose(fd_dirent_cue);

  delete[] buffer;
}

void DumpDirectoryPFS(int fd, int buffer_size, s64 offset) {
  char* buffer = new char[buffer_size] {0};

  std::string file_basename = "dump_" + std::to_string(buffer_size) + '+' + std::to_string(offset);

  fs::path read_path       = "/data/enderman/dump_pfs_read/read_" + file_basename + ".bin";
  fs::path dirent_path     = "/data/enderman/dump_pfs_getdents/dirent_" + file_basename + ".bin";
  fs::path read_cue_path   = "/data/enderman/dump_pfs_read/read_" + file_basename + ".cue";
  fs::path dirent_cue_path = "/data/enderman/dump_pfs_getdents/dirent_" + file_basename + ".cue";

  LogTest("PFS directory, fd =", fd, "buffer size =", buffer_size, "starting offset =", offset);

  s64 tbr         = 0;
  u16 max_loops   = 0; // 65536 iterations lmao
  int fd_read     = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int fd_read_cue = sceKernelOpen(read_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops) {
    tbr = DumpByRead(fd, fd_read, buffer, buffer_size);
    sceKernelWrite(fd_read_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
    if (tbr <= 0) break;
  }
  if (0 == max_loops) LogError("Aborted");
  sceKernelClose(fd_read);
  sceKernelClose(fd_read_cue);

  s64 idx           = 0;
  max_loops         = 0;
  int fd_dirent     = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  int fd_dirent_cue = sceKernelOpen(dirent_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
  while (--max_loops) {
    tbr = DumpByDirent(fd, fd_dirent, buffer, buffer_size, &idx);
    sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
    sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&idx), sizeof(s64) / sizeof(u8));
    if (tbr <= 0) break;
  }
  if (0 == max_loops) LogError("Aborted");
  sceKernelClose(fd_dirent);
  sceKernelClose(fd_dirent_cue);

  delete[] buffer;
}

TEST_GROUP (DirentTests) {
  int  fd;
  void setup() {}
  void teardown() {
    sceKernelClose(fd);
    fd = -1;
  }
};

TEST(DirentTests, DumpPFSRead) {
  const char*    app0_path        = "/app0/assets/misc";
  const fs::path output_path_base = "/data/enderman/dump_pfs_read";
  std::string    output_file_base = "dump_PFS_read_";
  std::string    output_file {};

  for (auto read_size: read_sizes) {
    for (auto read_offset: read_offsets) {
      output_file = output_file_base + std::to_string(read_size) + "+" + std::to_string(read_offset) + ".bin";

      fd         = sceKernelOpen(app0_path, O_RDONLY | O_DIRECTORY, 0777);
      int fd_out = sceKernelOpen(output_file.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0777);

      sceKernelClose(fd_out);
    }
  }
}

TEST(DirentTests, DumpPFSGetDents) {}

TEST(DirentTests, LseekRegularTests) {
  fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd, >, 0, "Unable to open /data/enderman");

  int status;

  errno  = 0;
  status = sceKernelLseek(fd, 0, 0);
  LONGLONGS_EQUAL_TEXT(0, status, "START+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -123, 0);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "START-123");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 0);
  LONGLONGS_EQUAL_TEXT(123456, status, "START+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 60, 0);
  LONGLONGS_EQUAL_TEXT(60, status, "START+60");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 1);
  LONGLONGS_EQUAL_TEXT(60, status, "CUR+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 24, 1);
  LONGLONGS_EQUAL_TEXT(84, status, "CUR+24");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -24, 1);
  LONGLONGS_EQUAL_TEXT(60, status, "CUR-24");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -6666, 1);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "CUR-6666");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 1);
  LONGLONGS_EQUAL_TEXT(123516, status, "CUR+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 2);
  LONGLONGS_EQUAL_TEXT(5120, status, "END+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 2);
  LONGLONGS_EQUAL_TEXT(128576, status, "END+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 100, 2);
  LONGLONGS_EQUAL_TEXT(5220, status, "END+100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100, 2);
  LONGLONGS_EQUAL_TEXT(5020, status, "END-100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100000, 2);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");
  UNSIGNED_INT_EQUALS(EINVAL, errno);
}

TEST(DirentTests, LseekPFSTests) {
  fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd, >, 0, "Unable to open /app0/assets/misc");

  s64 status;

  errno  = 0;
  status = sceKernelLseek(fd, 0, 0);
  LONGLONGS_EQUAL_TEXT(0, status, "START+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -123, 0);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "START-123");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 0);
  LONGLONGS_EQUAL_TEXT(123456, status, "START+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 60, 0);
  LONGLONGS_EQUAL_TEXT(60, status, "START+60");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 1);
  LONGLONGS_EQUAL_TEXT(60, status, "CUR+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 24, 1);
  LONGLONGS_EQUAL_TEXT(84, status, "CUR+24");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -24, 1);
  LONGLONGS_EQUAL_TEXT(60, status, "CUR-24");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -6666, 1);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "CUR-6666");
  UNSIGNED_INT_EQUALS(EINVAL, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 1);
  LONGLONGS_EQUAL_TEXT(123516, status, "CUR+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 0, 2);
  LONGLONGS_EQUAL_TEXT(65536, status, "END+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 2);
  LONGLONGS_EQUAL_TEXT(188992, status, "END+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 100, 2);
  LONGLONGS_EQUAL_TEXT(65636, status, "END+100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100, 2);
  LONGLONGS_EQUAL_TEXT(65436, status, "END-100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100000, 2);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");
  UNSIGNED_INT_EQUALS(EINVAL, errno);
}