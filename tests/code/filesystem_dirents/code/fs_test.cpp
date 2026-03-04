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

s64 dump_clone_read(std::vector<std::string>& dump_vector, fs::path& dump_path, oi::PfsDirent* dirent) {
  if (is_directory_relatives(dirent->d_name)) return 0;
  std::string filename           = std::string(dirent->d_name, dirent->d_namlen);
  fs::path    dump_path_full     = dump_path / filename;
  const char* dump_path_full_ptr = dump_path_full.c_str();
  dump_vector.emplace_back(filename);

  if (dirent->d_type == 2) {
    s64 fd_tmp = sceKernelOpen(dump_path_full_ptr, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd_tmp < 0) return fd_tmp;
    return sceKernelClose(fd_tmp);
  } else if (dirent->d_type == 4)
    return sceKernelMkdir(dump_path_full_ptr, 0777);
  return 0;
}

s64 dump_clone_getdirentries(std::vector<std::string>& dump_vector, fs::path& dump_path, oi::FolderDirent* dirent) {
  if (is_directory_relatives(dirent->d_name)) return 0;
  std::string filename           = std::string(dirent->d_name, dirent->d_namlen);
  fs::path    dump_path_full     = dump_path / filename;
  const char* dump_path_full_ptr = dump_path_full.c_str();
  dump_vector.emplace_back(filename);

  if (dirent->d_type == 8) {
    s64 fd_tmp = sceKernelOpen(dump_path_full_ptr, O_CREAT | O_TRUNC | O_WRONLY, 0777);
    if (fd_tmp < 0) return fd_tmp;
    return sceKernelClose(fd_tmp);
  } else if (dirent->d_type == 4)
    return sceKernelMkdir(dump_path_full_ptr, 0777);
  return 0;
}

void iterate_pfs_read(std::vector<std::string>& dump_vector, const char* source_path, fs::path& dump_path,
                      s64 (*dump_clone)(std::vector<std::string>& dump_vector, fs::path& dump_path, oi::PfsDirent* dirent) = nullptr) {
  constexpr int clone_buffer_size = 1024;
  char          clone_buffer[clone_buffer_size] {0};

  int fd_src {0};
  s64 read_start_position {0};
  u32 entry_offset {0};
  s64 bytes_read {0};
  u16 _canary {0};

  oi::PfsDirent* dirent_pfs {0};

  memset(clone_buffer, 0, clone_buffer_size);

  //

  fd_src = sceKernelOpen(source_path, O_RDONLY | O_DIRECTORY, 0777);
  do {
    entry_offset        = 0;
    read_start_position = sceKernelLseek(fd_src, 0, 1);
    bytes_read          = sceKernelRead(fd_src, clone_buffer, clone_buffer_size);
    //   Log("Read:", bytes_read, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
    if (bytes_read <= 0) break;

    dirent_pfs = reinterpret_cast<oi::PfsDirent*>(clone_buffer);
    while (dirent_pfs->d_reclen > 0) {
      // get current dirent, see what we have
      // Log("start:", read_start_position, "+", entry_offset, "fileno:", dirent_pfs->d_fileno, "name:", dirent_pfs->d_name, "type:", dirent_pfs->d_type,
      //     "namelen:", static_cast<u16>(dirent_pfs->d_namlen), "reclen:", dirent_pfs->d_reclen);

      // current dirent is >=24 bytes but OOB
      if ((entry_offset + dirent_pfs->d_reclen) > bytes_read) {
        break;
      }

      // current dirent fits
      if (dump_clone) dump_clone(dump_vector, dump_path, dirent_pfs);
      entry_offset += dirent_pfs->d_reclen;

      // peek at next entry, if less than 24 bytes are available, break
      dirent_pfs = reinterpret_cast<oi::PfsDirent*>(clone_buffer + entry_offset);
      if ((bytes_read - (entry_offset + 1)) < 24) {
        // Log("Not enough data available at", entry_offset);
        break;
      }
    }

    // unless entry offset moved by bytes read, we didn't consume the entire buffer
    if (((entry_offset + 1) != bytes_read) && (entry_offset > 0)) {
      // if we're here, then we did not. back off to the end of the last dirent
      sceKernelLseek(fd_src, read_start_position + entry_offset, 0);
    }
    // Log("End position:", entry_offset, "/", clone_buffer_size);
  } while (--_canary);

  // Log("Dump ended with status", bytes_read);
  sceKernelClose(fd_src);
}

void iterate_pfs_getdirents(std::vector<std::string>& dump_vector, const char* source_path, fs::path& dump_path,
                            s64 (*dump_clone)(std::vector<std::string>& dump_vector, fs::path& dump_path, oi::FolderDirent* dirent)) {
  constexpr int clone_buffer_size = 1024;
  char          clone_buffer[clone_buffer_size] {0};

  int fd_src {0};
  s64 read_start_position {0};
  u32 entry_offset {0};
  s64 bytes_read {0};
  s64 basep {0};
  u16 _canary {0};

  oi::FolderDirent* dirent_normal {0};

  memset(clone_buffer, 0, clone_buffer_size);

  fd_src = sceKernelOpen(source_path, O_RDONLY | O_DIRECTORY, 0777);

  do {
    read_start_position = sceKernelLseek(fd_src, 0, 1);
    bytes_read          = sceKernelGetdirentries(fd_src, clone_buffer, clone_buffer_size, &basep);
    // Log("Read:", bytes_read, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
    if (bytes_read <= 0) break;

    dirent_normal = reinterpret_cast<oi::FolderDirent*>(clone_buffer);
    // Log("start:", read_start_position, "+", entry_offset, "fileno:", dirent_normal->d_fileno, "name:", dirent_normal->d_name,
    //     "type:", static_cast<u16>(dirent_normal->d_type), "namelen:", static_cast<u16>(dirent_normal->d_namlen), "reclen:", dirent_normal->d_reclen);
    for (entry_offset = 0; (entry_offset < bytes_read) && (dirent_normal->d_reclen > 0); entry_offset += dirent_normal->d_reclen) {
      dirent_normal = reinterpret_cast<oi::FolderDirent*>(clone_buffer + entry_offset);

      if (dump_clone) dump_clone(dump_vector, dump_path, dirent_normal);
    }
    // Log("End position:", entry_offset, "/", clone_buffer_size);
  } while (--_canary);

  // Log("Dump ended with status", bytes_read);
  sceKernelClose(fd_src);
}

TEST(DirentTests, CompareDirentsAPP0) {
  LogTest("Compare PFS read&getdirentries, clone to /data");

  ///
  /// Setup dumping directory
  ///

  const char* directory_source      = "/app0/assets/misc";
  fs::path    clone_target_getdents = "/data/enderman/clone_getdents";
  fs::path    clone_target_read     = "/data/enderman/clone_read";

  std::vector<std::string> entries_read {};
  std::vector<std::string> entries_getdirentries {};

  sceKernelMkdir(clone_target_read.c_str(), 0777);
  sceKernelMkdir(clone_target_getdents.c_str(), 0777);

  //

  LogTest("Clone PFS (read) to", clone_target_read);
  iterate_pfs_read(entries_read, directory_source, clone_target_read, dump_clone_read);
  LogTest("Clone PFS (getdirentries) to", clone_target_getdents);
  iterate_pfs_getdirents(entries_getdirentries, directory_source, clone_target_getdents, dump_clone_getdirentries);

  std::sort(entries_getdirentries.begin(), entries_getdirentries.end());
  std::sort(entries_read.begin(), entries_read.end());

  CHECK_TRUE_TEXT(std::equal(entries_getdirentries.begin(), entries_getdirentries.end(), entries_read.begin(), entries_read.end()), "File lists are not equal");
}

TEST(DirentTests, LseekRegularTests) {
  LogTest("Testing lseek() on normal directory");

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
  LONGLONGS_EQUAL_TEXT(512, status, "END+0");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 123456, 2);
  LONGLONGS_EQUAL_TEXT(123968, status, "END+123456");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, 100, 2);
  LONGLONGS_EQUAL_TEXT(612, status, "END+100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100, 2);
  LONGLONGS_EQUAL_TEXT(412, status, "END-100");
  UNSIGNED_INT_EQUALS(0, errno);

  errno  = 0;
  status = sceKernelLseek(fd, -100000, 2);
  UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");
  UNSIGNED_INT_EQUALS(EINVAL, errno);
}

TEST(DirentTests, LseekPFSTests) {
  LogTest("Testing lseek() on PFS directory");

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