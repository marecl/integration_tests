#include "fs_test.h"

#include "fs_test_constants.h"
#include "orbis/UserService.h"

#include <CppUTest/TestHarness.h>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

const char* clone_source_app0               = "/app0/assets/misc";
const char* clone_destination_read          = "/data/enderman/clone_read";
const char* clone_destination_getdirentries = "/data/enderman/clone_getdents";

namespace fs = std::filesystem;
namespace oi = OrbisInternals;

// s64  DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size);
// s64  DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx);
// void DumpDirectory(int fd, int buffer_size, s64 offset);
// void DumpDirectoryPFS(int fd, int buffer_size, s64 offset);

bool PrepareTests() {
  fs::path clone_dir = "/data/enderman/clone";
  fs::path target {};
  s32      status {};

  RegenerateDir("/data/enderman");
  sceKernelMkdir(clone_dir.c_str(), 0777);

  for (auto& dent: fs::directory_iterator("/app0/assets/misc")) {
    target = clone_dir / dent.path().filename();
    if (dent.is_regular_file()) {
      status = touch(target.c_str());
      continue;
    }
    if (dent.is_directory()) {
      status = sceKernelMkdir(target.c_str(), 0777);
      continue;
    }
    LogError("Can't create", target.string());
    return false;
  }
  return true;
}

s64 undump_file(const char* path, char* data, u64 length) {
  int fd = sceKernelOpen(path, O_RDONLY, 0777);
  if (fd < 0) return fd;
  memset(data, 'A', length);
  int tbr = sceKernelRead(fd, data, length);
  if (tbr < 0) return tbr;
  if (auto res = sceKernelClose(fd); res < 0) return res;
  return tbr;
}

TEST_GROUP (DirentTests) {
  const char* input_pfs                   = "/app0/assets/misc";
  const char* input_normal                = "/data/enderman/clone";
  const char* output_dir                  = "/data/enderman/dump";
  const char* output_pfs_read             = "/data/enderman/dump/pfs_read.bin";
  const char* output_pfs_getdirentries    = "/data/enderman/dump/pfs_getdirent.bin";
  const char* output_normal_read          = "/data/enderman/dump/normal_read.bin";
  const char* output_normal_getdirentries = "/data/enderman/dump/normal_getdirent.bin";

  bool everything_dumped = false;

  int fd;
  s64 tbr;

  void setup() {
    fd  = -1;
    tbr = 0;
  }
  void teardown() {
    sceKernelClose(fd);
    fd = -1;
  }
};

TEST(DirentTests, PFSGetdirentries) {
  LogTest("<<<< PFS getdirentries tests >>>>");

  fs::path output_root = "/data/enderman/pfs_getdirentries";
  char     buffer[65536];
  int      result_cast {};

  fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
  s64 basep {};
  for (auto& spec: pfs_dirent_variants) {
    basep = 0;
    memset(buffer, 0xAA, 65536);
    result_cast = int(spec.expected_result);

    if (spec.read_offset >= 0) CHECK_EQUAL(spec.read_offset, sceKernelLseek(fd, spec.read_offset, 0));
    errno = 0;
    tbr   = sceKernelGetdirentries(fd, buffer, spec.read_size, &basep);
    LogTest(spec.read_size, spec.read_offset, spec.expected_basep, result_cast, spec.expected_errno, "\t->\t", basep, tbr, errno, "\t",
            to_hex_string(buffer, 16, ""));
    if (tbr < 0) {
      CHECK_EQUAL(result_cast, tbr);
    } else {
      CHECK_EQUAL(spec.expected_result, tbr);
    }
    CHECK_EQUAL(spec.expected_errno, errno);
    CHECK_EQUAL(spec.expected_basep, basep);
    // dump good ones to file
  }
  sceKernelClose(fd);
}

TEST(DirentTests, NormalGetdirentries) {
  LogTest("<<<< Normal getdirentries tests >>>>");

  char buffer[65536];
  int  result_cast {};

  fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
  s64 basep {};
  for (auto& spec: normal_dirent_variants) {
    basep = 0;
    memset(buffer, 0xAA, 65536);
    result_cast = int(spec.expected_result);

    if (spec.read_offset >= 0) CHECK_EQUAL(spec.read_offset, sceKernelLseek(fd, spec.read_offset, 0));
    errno = 0;
    tbr   = sceKernelGetdirentries(fd, buffer, spec.read_size, &basep);
    LogTest(spec.read_size, spec.read_offset, spec.expected_basep, result_cast, spec.expected_errno, "\t->\t", basep, tbr, errno,
            to_hex_string(buffer, 16, ""));
    if (tbr < 0) {
      CHECK_EQUAL(result_cast, tbr);
    } else {
      CHECK_EQUAL(spec.expected_result, tbr);
    }
    CHECK_EQUAL(spec.expected_errno, errno);
    CHECK_EQUAL(spec.expected_basep, basep);
    // dump good ones to file
  }
  sceKernelClose(fd);
}

s64 compare_data_dump(const void* master, const void* test, s64 buffer_size, s64 tbr, struct oi::DirentCombinationRead* spec) {
  const char* master_data = reinterpret_cast<const char*>(master) + spec->read_offset;
  const char* test_data   = reinterpret_cast<const char*>(test);

  if (auto qw = imemcmp(master_data, test_data, tbr); qw != -1) {
    LogError("Inconsistent read at", spec->read_offset + qw);
    LogError("Global dump:", to_hex_string(master_data + qw, std::min((s64)48, tbr)));
    LogError("Recent dump:", to_hex_string(test_data + qw, std::min((s64)48, tbr)));

    return -1;
  }
  return 0;
}

TEST(DirentTests, PFSRead) {
  LogTest("<<<< PFS read tests >>>>");

  char      buffer[65536];
  char      master_buffer[65536];
  const s64 master_length = undump_file(output_pfs_read, master_buffer, 65536);
  s64       expected_length_adj {};

  LogTest("Master PFS read length is", master_length);

  fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
  for (auto& spec: normal_read_variants) {
    memset(buffer, 'A', 65536);

    CHECK_EQUAL(spec.read_offset, sceKernelLseek(fd, spec.read_offset, 0));
    errno = 0;
    tbr   = sceKernelRead(fd, buffer, spec.read_size);

    expected_length_adj = (master_length - spec.read_offset) > 0 ? master_length - spec.read_offset : 0;
    expected_length_adj = std::min(expected_length_adj, spec.read_size);

    LogTest(spec.read_size, spec.read_offset, int(spec.expected_result), spec.expected_errno, "\t->\t", tbr, errno, expected_length_adj, "\t",
            to_hex_string(buffer, 16, ""));
    CHECK_EQUAL(expected_length_adj, tbr);
    CHECK_EQUAL(spec.expected_errno, errno);
    compare_data_dump(master_buffer, buffer, 65536, tbr, &spec);
    // dump good ones to file
  }
  sceKernelClose(fd);
}

TEST(DirentTests, NormalRead) {
  LogTest("<<<< Normal read tests >>>>");

  char      buffer[65536];
  char      master_buffer[65536];
  const s64 master_length = undump_file(output_normal_read, master_buffer, 65536);
  s64       expected_length_adj {};

  LogTest("Master PFS read length is", master_length);

  fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
  for (auto& spec: normal_read_variants) {
    memset(buffer, 'A', 65536);

    CHECK_EQUAL(spec.read_offset, sceKernelLseek(fd, spec.read_offset, 0));
    errno = 0;
    tbr   = sceKernelRead(fd, buffer, spec.read_size);

    expected_length_adj = (master_length - spec.read_offset) > 0 ? master_length - spec.read_offset : 0;
    expected_length_adj = std::min(expected_length_adj, spec.read_size);

    LogTest(spec.read_size, spec.read_offset, int(spec.expected_result), spec.expected_errno, "\t->\t", tbr, errno, expected_length_adj, "\t",
            to_hex_string(buffer, 16, ""));
    CHECK_EQUAL(expected_length_adj, tbr);
    CHECK_EQUAL(spec.expected_errno, errno);
    compare_data_dump(master_buffer, buffer, 65536, tbr, &spec);
    // dump good ones to file
  }
  sceKernelClose(fd);
}

TEST(DirentTests, NormalGetdirentriesErrors) {
  LogTest("<<<< Normal getdirentries test illegal reads >>>>");
  LogTest("Test reads that do not break 512b alignment");

  char buffer[1024];
  int  result_cast {};
  s64  spec_read_size {};
  s64  spec_offset_base {};
  s64  spec_offset {};
  s64  basep {};
  memset(buffer, 0xAA, 1024);

  fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);

  for (spec_offset_base = 0; spec_offset_base < 65536; spec_offset_base += 512) {
    for (spec_read_size = 0; spec_read_size < 512; ++spec_read_size) {
      basep       = 0;
      spec_offset = spec_offset_base + 511 - spec_read_size;
      CHECK_EQUAL(spec_offset, sceKernelLseek(fd, spec_offset, 0));
      errno = 0;
      tbr   = sceKernelGetdirentries(fd, buffer, spec_read_size, &basep);
      CHECK_EQUAL(0xAAAAAAAAAAAAAAAA, *reinterpret_cast<u64*>(buffer));
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, int(tbr));
      CHECK_EQUAL(EINVAL, errno);
      CHECK_EQUAL(0, basep);
    }
  }
  sceKernelClose(fd);
}

TEST(DirentTests, PFSGetdirentriesErrors) {
  LogTest("<<<< PFS getdirentries test illegal reads >>>>");
  LogTest("Test reads that do not break 512b alignment");

  char buffer[1024];
  int  result_cast {};
  s64  spec_read_size {};
  s64  spec_offset_base {};
  s64  spec_offset {};
  s64  basep {};
  memset(buffer, 0xAA, 1024);

  fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);

  for (spec_offset_base = 0; spec_offset_base < 65536; spec_offset_base += 512) {
    for (spec_read_size = 0; spec_read_size < 512; ++spec_read_size) {
      basep       = 0;
      spec_offset = spec_offset_base + 511 - spec_read_size;
      CHECK_EQUAL(spec_offset, sceKernelLseek(fd, spec_offset, 0));
      errno = 0;
      tbr   = sceKernelGetdirentries(fd, buffer, spec_read_size, &basep);
      CHECK_EQUAL(0xAAAAAAAAAAAAAAAA, *reinterpret_cast<u64*>(buffer));
      CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, int(tbr));
      CHECK_EQUAL(EINVAL, errno);
      CHECK_EQUAL(0, basep);
    }
  }
  sceKernelClose(fd);
}

s64 NormalComparator(const char* read, const char* getdirentries, u64 length) {
  s64 offset {0};
  while (offset < length) {
    const oi::FolderDirent* dirent_read          = reinterpret_cast<const oi::FolderDirent*>(read + offset);
    const oi::FolderDirent* dirent_getdirentries = reinterpret_cast<const oi::FolderDirent*>(getdirentries + offset);

    if (dirent_read->d_namlen != dirent_getdirentries->d_namlen) break;
    if (dirent_read->d_reclen != dirent_getdirentries->d_reclen) break;
    if (dirent_read->d_type != dirent_getdirentries->d_type) break;
    if (memcmp(dirent_read->d_name, dirent_getdirentries->d_name, dirent_read->d_namlen)) break; // namlen is the same
    offset += dirent_read->d_reclen;
    if (dirent_read->d_reclen == 0) break;
  }
  return offset;
}

TEST(DirentTests, Normal_Consistency) {
  LogTest("<<<< Normal read and getdirentries consistency >>>>");

  char master_read[65536] {'A'};
  char master_getdirentries[65536] {'A'};

  auto master_read_size          = undump_file(output_normal_read, master_read, 65536);
  auto master_getdirentries_size = undump_file(output_normal_getdirentries, master_getdirentries, 65536);

  if (auto res = NormalComparator(master_read, master_getdirentries, master_read_size); master_getdirentries_size != res) {
    LogError("Normal read and getdirentries have a different dirent at", res, ":");
    LogError("Normal Read:\t   ", to_hex_string(master_read + res, 24));
    LogError("Normal Getdirentries: ", to_hex_string(master_read + res, 24));
    FAIL("Normal read and getdirentries returned different amount of data");
  }
}

s64 PFSComparator(const char* read, const char* getdirentries, u64 length) {

  s64 offset {0};
  while (offset < length) {
    const oi::PfsDirent*    dirent_read          = reinterpret_cast<const oi::PfsDirent*>(read + offset);
    const oi::FolderDirent* dirent_getdirentries = reinterpret_cast<const oi::FolderDirent*>(getdirentries + offset);

    if (dirent_read->d_namlen != dirent_getdirentries->d_namlen) break;
    if (dirent_read->d_reclen != dirent_getdirentries->d_reclen) break;
    // if (dirent_read->d_type != dirent_getdirentries->d_type) break;
    if (memcmp(dirent_read->d_name, dirent_getdirentries->d_name, dirent_read->d_namlen)) break; // namlen is the same
    if (dirent_read->d_reclen == 0) break;                                                       // reclen is the same
    offset += dirent_read->d_reclen;
    if (dirent_read->d_reclen == 0) break;
  }
  return offset;
}

TEST(DirentTests, PFS_Consistency) {
  LogTest("<<<< PFS read and getdirentries consistency >>>>");
  LogWarning("This function does not translate d_type between PFS and user filesystems");

  char master_read[65536] {'A'};
  char master_getdirentries[65536] {'A'};

  auto master_read_size          = undump_file(output_pfs_read, master_read, 65536);
  auto master_getdirentries_size = undump_file(output_pfs_getdirentries, master_getdirentries, 65536);

  if (auto res = PFSComparator(master_read, master_getdirentries, 65536); master_getdirentries_size != res) {
    LogError("PFS read and getdirentries have a different dirent at", res, ":");
    LogError("PFS Read:\t   ", to_hex_string(master_read + res, 24));
    LogError("PFS Getdirentries: ", to_hex_string(master_read + res, 24));
    FAIL("PFS read and getdirentries returned different amount of data");
  }
}

TEST(DirentTests, DumpEverythingRaw) {
  LogTest("<<<< Dump everything >>>>");

  int  fd_read {};
  int  fd_dump {};
  char buffer[65536];

  RegenerateDir(output_dir);

  //

  fd_read = sceKernelOpen(input_pfs, O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd_read, >, 0, "Can't open input dir");

  //

  fd_dump = sceKernelOpen(output_pfs_read, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelRead(fd_read, buffer, 65536);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "PFS read failed");
    LogTest("PFS read got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(65536, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  fd_dump = sceKernelOpen(output_pfs_getdirentries, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  CHECK_EQUAL_ZERO_TEXT(sceKernelLseek(fd_read, 0, 0), "Can't rewind directory");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelGetdirentries(fd_read, buffer, 65536, nullptr);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "PFS sceKernelGetdirentries failed");
    LogTest("PFS sceKernelGetdirentries got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(10616, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  CHECK_EQUAL_ZERO(sceKernelClose(fd_read));
  fd_read = sceKernelOpen(input_normal, O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd_read, >, 0, "Can't open input dir");

  //

  fd_dump = sceKernelOpen(output_normal_read, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelRead(fd_read, buffer, 65536);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "Normal read failed");
    LogTest("Normal read got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(8704, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //
  fd_dump = sceKernelOpen(output_normal_getdirentries, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  CHECK_EQUAL_ZERO_TEXT(sceKernelLseek(fd_read, 0, 0), "Can't rewind directory");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelGetdirentries(fd_read, buffer, 65536, nullptr);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "Normal sceKernelGetdirentries failed");
    LogTest("Normal sceKernelGetdirentries got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(8704, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  CHECK_EQUAL_ZERO(sceKernelClose(fd_read));

  everything_dumped = true;
}

// TEST(DirentTests, DirentPFSGetdirentries) {
//   LogTest("<<<< PFS getdirentries basic test >>>>");

//   const char* dir_test = "/app0/assets/misc";

//   s32           tbr {};
//   constexpr u32 buffer_size   = 65536;
//   u32           view_size     = 48;
//   u32           view_size_end = 24;
//   u32           zero          = 0;
//   char          buffer[buffer_size] {0};
//   char          reflection[buffer_size] {0};

//   auto quickprint = [&view_size, &view_size_end](std::string title, const void* array, size_t size) -> void {
//     std::string out {title};
//     for (u32 idx = 0; idx < view_size; ++idx) {
//       if (!(idx % 12)) out += "\r\n\t\t\t\t\t\t";
//       auto hexed = to_hex<u16>(*(reinterpret_cast<const u8*>(array) + idx));
//       out += (hexed.length() == 1 ? "0" : "") + hexed + " ";
//     }
//     if (0 == view_size_end) {
//       Log(out);
//       return;
//     }
//     out += "\r\n\t\t\t\t\t\t----------";
//     for (u32 idx = 0; idx < view_size_end; ++idx) {
//       if (!(idx % 12)) out += "\r\n\t\t\t\t\t\t";
//       auto hexed = to_hex<u16>(*(reinterpret_cast<const u8*>(array) + size - view_size_end + idx));
//       out += (hexed.length() == 1 ? "0" : "") + hexed + " ";
//     }
//     Log(out);
//   };

//   auto rd = [bs = &buffer_size](s32 fd, void* buffer, s64 size, s64 offset) -> s64 {
//     auto buffer_ptr = static_cast<char*>(buffer);
//     errno           = 0;
//     CHECK_EQUAL(offset, sceKernelLseek(fd, offset, 0));
//     CHECK_EQUAL_ZERO(errno);
//     memset(buffer_ptr, 0xAA, *bs);
//     errno = 0;
//     return sceKernelGetdirentries(fd, buffer_ptr, size, nullptr);
//   };

//   // clang-format off
//   errno = 0;  fd    = sceKernelOpen(dir_test, O_RDONLY, 0777);  CHECK_COMPARE(0, <, fd);

//   LogTest("EINVAL when size+offset falls before first 512 bytes");
//   for(u16 len=0; len < 512; len+=1){
//     tbr = rd(fd,buffer, len, 511-len);
//     if(tbr >= 0)
//       LogError("Incorrect returns for read size", len, "with offset",511-len);
//     CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, tbr);
//     CHECK_EQUAL(EINVAL, errno); CHECK_TRUE(fillcheck(buffer, 0xAA, len));
//   }

//   view_size  = 48;  view_size_end = 48;
//   tbr = rd(fd, buffer, 512, 0);
//   CHECK_EQUAL(496, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+0 R512", buffer,512);
//   CHECK_EQUAL_TEXT(tbr, validate_pfs_getdirentries(buffer, tbr),"Direntries are likely corrupted");
//   *(reinterpret_cast<u32*>(buffer))=0;
//   *(reinterpret_cast<u32*>(buffer+24))=0;
//   CHECK_EQUAL_TEXT(-1, imemcmp(buffer, pfs_dirent_entry_dot, 24), "[.] failed");
//   CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 24, pfs_dirent_entry_dotdot, 24), "[..] failed");

//   for (auto idx = 1; idx < 24; idx++){
//     tbr = rd(fd, reflection, 512, idx); CHECK_EQUAL(472, tbr);  CHECK_EQUAL_ZERO(errno);
//     CHECK_EQUAL_TEXT(tbr, validate_pfs_getdirentries(reflection, tbr),"Direntries are likely corrupted");
//     *(reinterpret_cast<u32*>(reflection))=0;
//     CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 24, reflection, tbr), "memory compare failed");
//   }

//   view_size=32;view_size_end=32;
//   tbr = rd(fd, buffer, 600, 0); quickprint("\t+0 R600", buffer,600); CHECK_EQUAL(496, tbr);
//   CHECK_EQUAL(tbr, validate_pfs_getdirentries(buffer, tbr));

//   tbr = rd(fd, buffer, 700, 0); quickprint("\t+0 R700", buffer,700); CHECK_EQUAL(496, tbr);
//   CHECK_EQUAL(tbr, validate_pfs_getdirentries(buffer, tbr));

//   tbr = rd(fd, buffer, 800, 0); quickprint("\t+0 R800", buffer,800); CHECK_EQUAL(496, tbr);
//   CHECK_EQUAL(tbr, validate_pfs_getdirentries(buffer, tbr));

//   tbr = rd(fd, buffer, 900, 0); quickprint("\t+0 R900", buffer,900); CHECK_EQUAL(496, tbr);
//   CHECK_EQUAL(tbr, validate_pfs_getdirentries(buffer, tbr));

//   //

//   view_size  = 48;  view_size_end = 48;
//   tbr = rd(fd, buffer, 1023, 0);
//   CHECK_EQUAL(496, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+0 R1023", buffer,1023);
//   CHECK_EQUAL_TEXT(tbr, validate_pfs_getdirentries(buffer, tbr),"Direntries are likely corrupted");
//   *(reinterpret_cast<u32*>(buffer))=0;  *(reinterpret_cast<u32*>(buffer+24))=0;
//   CHECK_EQUAL_TEXT(-1, imemcmp(buffer, pfs_dirent_entry_dot, 24), "[.] failed");
//   CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 24, pfs_dirent_entry_dotdot, 24), "[..] failed");

//   for (auto idx = 1; idx < 24; idx++){
//     tbr = rd(fd, reflection, 1023, idx); CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);
//     CHECK_EQUAL_TEXT(tbr, validate_pfs_getdirentries(reflection, tbr),"Direntries are likely corrupted");
//     *(reinterpret_cast<u32*>(reflection))=0;
//     CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 24, reflection, 472), "memory compare failed");
//   }

//   // comment below is unrelated to what's below that comment

//   // view_size  = 24;  view_size_end = 64;
//   // okay, this is complicated. first dirent is always rounded up to the nearest upper one, so for any offset <=24 first dirent presented is going to be [..]
//   // this is subtracted in favour of possibly populating next dirent (doesn't happen here).
//   // differences between larger buffer and smaller occur between [..] and last entry, so real comparsion should happen between 24 and 496 (472 length)
//   tbr = rd(fd, buffer, 1024, 0);        CHECK_EQUAL(1016, tbr); CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R1024", buffer,1024);
//   *(reinterpret_cast<u32*>(buffer))=0;  *(reinterpret_cast<u32*>(buffer+24))=0;
//   tbr = rd(fd, reflection, 1024, 5);    CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R1024", reflection,1024);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 24, reflection, 992), "memory compare failed"); tbr = rd(fd, reflection, 1024, 6);    CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R1024", reflection,1024);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 24, reflection, 992), "memory compare failed"); tbr = rd(fd,
//   reflection, 1024, 7);    CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R1024", reflection,1024);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 24, reflection, 992), "memory compare failed");

//   tbr = rd(fd, buffer, 1025, 0);        CHECK_EQUAL(1016, tbr); CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R1025", buffer,1025);
//   *(reinterpret_cast<u32*>(buffer))=0;  *(reinterpret_cast<u32*>(buffer+24))=0;
//   tbr = rd(fd, reflection, 1025, 5);    CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R1025", reflection,1025);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 24, reflection, 992), "memory compare failed"); tbr = rd(fd, reflection, 1025, 6);    CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R1025", reflection,1025);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 24, reflection, 992), "memory compare failed"); tbr = rd(fd,
//   reflection, 1025, 7);    CHECK_EQUAL(992, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R1025", reflection,1025);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 24, reflection, 992), "memory compare failed");

//   // clang-format on

//   CHECK_EQUAL_ZERO(sceKernelClose(fd));
// }

// s64 DumpByRead(int dir_fd, int dump_fd, char* buffer, size_t size) {
//   memset(buffer, 0xAA, size);

//   s64 tbr = sceKernelRead(dir_fd, buffer, size);
//   // Log("Read got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1));

//   if (tbr < 0) {
//     LogError("Read finished with error:", tbr);
//     return tbr;
//   }

//   if (s64 tbw = sceKernelWrite(dump_fd, buffer, tbr); tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes");
//   return tbr;
// }

// s64 DumpByDirent(int dir_fd, int dump_fd, char* buffer, size_t size, s64* idx) {
//   // magic to determine how many trailing elements were cut
//   memset(buffer, 0xAA, size);

//   s64 tbr = sceKernelGetdirentries(dir_fd, buffer, size, idx);
//   // Log("Dirent got", tbr, "/", size, "bytes, ptr =", sceKernelLseek(dir_fd, 0, 1), "idx =", *idx);

//   if (tbr < 0) {
//     LogError("Dirent finished with error:", tbr);
//     return tbr;
//   }

//   if (s64 tbw = sceKernelWrite(dump_fd, buffer, tbr); tbw != tbr) LogError("Written", tbw, "bytes out of", tbr, "bytes");
//   return tbr;
// }

// void DumpDirectory(int fd, int buffer_size, s64 offset) {
//   char* buffer = new char[buffer_size] {0};

//   std::string file_basename = "dump_" + std::to_string(buffer_size) + '+' + std::to_string(offset);

//   fs::path read_path       = "/data/enderman/dump_data_read/" + file_basename + ".bin";
//   fs::path dirent_path     = "/data/enderman/dump_data_getdents/" + file_basename + ".bin";
//   fs::path read_cue_path   = "/data/enderman/dump_data_read/" + file_basename + ".cue";
//   fs::path dirent_cue_path = "/data/enderman/dump_data_getdents/" + file_basename + ".cue";

//   LogTest("normal", "directory, fd =", fd, "buffer size =", buffer_size, "starting offset =", offset);

//   s64 tbr         = 0;
//   u16 max_loops   = 0; // 65536 iterations lmao
//   int fd_read     = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   int fd_read_cue = sceKernelOpen(read_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
//   while (--max_loops) {
//     tbr = DumpByRead(fd, fd_read, buffer, buffer_size);
//     sceKernelWrite(fd_read_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
//     if (tbr <= 0) break;
//   }
//   if (0 == max_loops) LogError("Aborted");
//   sceKernelClose(fd_read);
//   sceKernelClose(fd_read_cue);

//   s64 idx           = 0;
//   max_loops         = 0;
//   int fd_dirent     = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   int fd_dirent_cue = sceKernelOpen(dirent_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
//   while (--max_loops) {
//     tbr = DumpByDirent(fd, fd_dirent, buffer, buffer_size, &idx);
//     sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
//     sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&idx), sizeof(s64) / sizeof(u8));
//     if (tbr <= 0) break;
//   }
//   if (0 == max_loops) LogError("Aborted");
//   sceKernelClose(fd_dirent);
//   sceKernelClose(fd_dirent_cue);

//   delete[] buffer;
// }

// void DumpDirectoryPFS(int fd, int buffer_size, s64 offset) {
//   char* buffer = new char[buffer_size] {0};

//   std::string file_basename = "dump_" + std::to_string(buffer_size) + '+' + std::to_string(offset);

//   fs::path read_path       = "/data/enderman/dump_pfs_read/read_" + file_basename + ".bin";
//   fs::path dirent_path     = "/data/enderman/dump_pfs_getdents/dirent_" + file_basename + ".bin";
//   fs::path read_cue_path   = "/data/enderman/dump_pfs_read/read_" + file_basename + ".cue";
//   fs::path dirent_cue_path = "/data/enderman/dump_pfs_getdents/dirent_" + file_basename + ".cue";

//   LogTest("PFS directory, fd =", fd, "buffer size =", buffer_size, "starting offset =", offset);

//   s64 tbr         = 0;
//   u16 max_loops   = 0; // 65536 iterations lmao
//   int fd_read     = sceKernelOpen(read_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   int fd_read_cue = sceKernelOpen(read_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
//   while (--max_loops) {
//     tbr = DumpByRead(fd, fd_read, buffer, buffer_size);
//     sceKernelWrite(fd_read_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
//     if (tbr <= 0) break;
//   }
//   if (0 == max_loops) LogError("Aborted");
//   sceKernelClose(fd_read);
//   sceKernelClose(fd_read_cue);

//   s64 idx           = 0;
//   max_loops         = 0;
//   int fd_dirent     = sceKernelOpen(dirent_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   int fd_dirent_cue = sceKernelOpen(dirent_cue_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
//   if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp != offset) LogError("Lseek failed:", _tmp);
//   while (--max_loops) {
//     tbr = DumpByDirent(fd, fd_dirent, buffer, buffer_size, &idx);
//     sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&tbr), sizeof(s64) / sizeof(u8));
//     sceKernelWrite(fd_dirent_cue, reinterpret_cast<u8*>(&idx), sizeof(s64) / sizeof(u8));
//     if (tbr <= 0) break;
//   }
//   if (0 == max_loops) LogError("Aborted");
//   sceKernelClose(fd_dirent);
//   sceKernelClose(fd_dirent_cue);

//   delete[] buffer;
// }

// OK, whole block
// applies for PFS read
// s64 dump_clone_pfs_read(const char* dump_path, oi::PfsDirent* dirent) {
//   std::string filename           = std::string(dirent->d_name, dirent->d_namlen);
//   fs::path    dump_path_full     = fs::path(dump_path) / filename;
//   const char* dump_path_full_ptr = dump_path_full.c_str();

//   if (dirent->d_type == 2) {
//     s64 fd_tmp = sceKernelOpen(dump_path_full_ptr, O_CREAT | O_TRUNC | O_WRONLY, 0777);
//     if (fd_tmp < 0) return fd_tmp;
//     return sceKernelClose(fd_tmp);
//   } else if (dirent->d_type == 4)
//     return sceKernelMkdir(dump_path_full_ptr, 0777);
//   return 0;
// }

// // applies for normal read+getdirentries, PFS getdirentries
// s64 dump_clone_read(const char* dump_path, oi::FolderDirent* dirent) {
//   std::string filename           = std::string(dirent->d_name, dirent->d_namlen);
//   fs::path    dump_path_full     = fs::path(dump_path) / filename;
//   const char* dump_path_full_ptr = dump_path_full.c_str();

//   if (dirent->d_type == 8) {
//     s64 fd_tmp = sceKernelOpen(dump_path_full_ptr, O_CREAT | O_TRUNC | O_WRONLY, 0777);
//     if (fd_tmp < 0) return fd_tmp;
//     return sceKernelClose(fd_tmp);
//   } else if (dirent->d_type == 4)
//     return sceKernelMkdir(dump_path_full_ptr, 0777);
//   return 0;
// }

// s64 iterate_pfs_read(const char* source_path, const std::function<s64(oi::PfsDirent*)>& entry_cb = nullptr) {
//   constexpr int clone_buffer_size = 1024;
//   char          clone_buffer[clone_buffer_size] {0};

//   int fd_src {0};
//   s64 read_start_position {0};
//   u32 entry_offset {0};
//   s64 bytes_read {0};
//   u16 _canary {0};

//   oi::PfsDirent* dirent {0};

//   memset(clone_buffer, 0, clone_buffer_size);

//   //

//   fd_src = sceKernelOpen(source_path, O_RDONLY | O_DIRECTORY, 0777);
//   if (fd_src < 0) return fd_src;

//   do {
//     entry_offset        = 0;
//     read_start_position = sceKernelLseek(fd_src, 0, 1);
//     if (read_start_position < 0) return read_start_position;
//     bytes_read = sceKernelRead(fd_src, clone_buffer, clone_buffer_size);
//     if (bytes_read < 0) return bytes_read;
//     //   Log("Read:", bytes_read, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
//     if (bytes_read <= 0) break;

//     dirent = reinterpret_cast<oi::PfsDirent*>(clone_buffer);
//     while (dirent->d_reclen > 0) {
//       // get current dirent, see what we have
//       // Log("start:", read_start_position, "+", entry_offset, "fileno:", dirent->d_fileno, "name:", dirent->d_name, "type:", dirent->d_type,
//       //     "namelen:", static_cast<u16>(dirent->d_namlen), "reclen:", dirent->d_reclen);

//       // current dirent is >=24 bytes but OOB
//       if ((entry_offset + dirent->d_reclen) > bytes_read) {
//         break;
//       }

//       // current dirent fits
//       if (entry_cb) {
//         if (s64 cb_result = entry_cb(dirent); cb_result <= 0) return cb_result;
//       }
//       entry_offset += dirent->d_reclen;

//       // peek at next entry, if less than 24 bytes are available, break
//       dirent = reinterpret_cast<oi::PfsDirent*>(clone_buffer + entry_offset);
//       if ((bytes_read - (entry_offset + 1)) < 24) {
//         // Log("Not enough data available at", entry_offset);
//         break;
//       }
//     }

//     // unless entry offset moved by bytes read, we didn't consume the entire buffer
//     if (((entry_offset + 1) != bytes_read) && (entry_offset > 0)) {
//       // if we're here, then we did not. back off to the end of the last dirent
//       sceKernelLseek(fd_src, read_start_position + entry_offset, 0);
//     }
//     // Log("End position:", entry_offset, "/", clone_buffer_size);
//   } while (--_canary);

//   // Log("Dump ended with status", bytes_read);
//   sceKernelClose(fd_src);
//   return 0;
// }

// s64 iterate_normal_read(const char* source_path, const std::function<s64(oi::FolderDirent*)>& entry_cb = nullptr) {
//   constexpr int clone_buffer_size = 1024;
//   char          clone_buffer[clone_buffer_size] {0};

//   int fd_src {0};
//   s64 read_start_position {0};
//   u32 entry_offset {0};
//   s64 bytes_read {0};
//   u16 _canary {0};

//   oi::FolderDirent* dirent {0};

//   memset(clone_buffer, 0, clone_buffer_size);

//   //

//   fd_src = sceKernelOpen(source_path, O_RDONLY | O_DIRECTORY, 0777);
//   do {
//     entry_offset        = 0;
//     read_start_position = sceKernelLseek(fd_src, 0, 1);
//     bytes_read          = sceKernelRead(fd_src, clone_buffer, clone_buffer_size);
//     //   Log("Read:", bytes_read, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
//     if (bytes_read <= 0) break;

//     dirent = reinterpret_cast<oi::FolderDirent*>(clone_buffer);
//     while (dirent->d_reclen > 0) {
//       // get current dirent, see what we have
//       // Log("start:", read_start_position, "+", entry_offset, "fileno:", dirent_pfs->d_fileno, "name:", dirent_pfs->d_name, "type:", dirent_pfs->d_type,
//       //     "namelen:", static_cast<u16>(dirent_pfs->d_namlen), "reclen:", dirent_pfs->d_reclen);

//       // current dirent is >=24 bytes but OOB
//       if ((entry_offset + dirent->d_reclen) >= bytes_read) {
//         break;
//       }

//       // current dirent fits
//       if (entry_cb) {
//         if (s64 cb_result = entry_cb(dirent); cb_result <= 0) return cb_result;
//       }
//       entry_offset += dirent->d_reclen;

//       // peek at next entry, if less than 24 bytes are available, break
//       dirent = reinterpret_cast<oi::FolderDirent*>(clone_buffer + entry_offset);
//       if ((bytes_read - (entry_offset + 1)) < 24) {
//         // Log("Not enough data available at", entry_offset);
//         break;
//       }
//     }

//     // unless entry offset moved by bytes read, we didn't consume the entire buffer
//     if (((entry_offset + 1) != bytes_read) && (entry_offset > 0)) {
//       // if we're here, then we did not. back off to the end of the last dirent
//       sceKernelLseek(fd_src, read_start_position + entry_offset, 0);
//     }
//     // Log("End position:", entry_offset, "/", clone_buffer_size);
//   } while (--_canary);

//   // Log("Dump ended with status", bytes_read);
//   sceKernelClose(fd_src);
//   return 0;
// }

// s64 iterate_getdirentries(const char* source_path, const std::function<s64(oi::FolderDirent*)>& entry_cb = nullptr) {
//   constexpr int clone_buffer_size = 1024;
//   char          clone_buffer[clone_buffer_size] {0};

//   int fd_src {0};
//   s64 read_start_position {0};
//   u32 entry_offset {0};
//   s64 bytes_read {0};
//   s64 basep {0};
//   u16 _canary {0};

//   oi::FolderDirent* dirent {0};

//   memset(clone_buffer, 0, clone_buffer_size);

//   fd_src = sceKernelOpen(source_path, O_RDONLY | O_DIRECTORY, 0777);
//   if (fd_src < 0) return -1;

//   do {
//     read_start_position = sceKernelLseek(fd_src, 0, 1);
//     bytes_read          = sceKernelGetdirentries(fd_src, clone_buffer, clone_buffer_size, &basep);
//     // Log("Read:", bytes_read, read_start_position, "-", sceKernelLseek(fd_src, 0, 1));
//     if (bytes_read <= 0) break;

//     dirent = reinterpret_cast<oi::FolderDirent*>(clone_buffer);
//     // Log("start:", read_start_position, "+", entry_offset, "fileno:", dirent->d_fileno, "name:", dirent->d_name,
//     //     "type:", static_cast<u16>(dirent->d_type), "namelen:", static_cast<u16>(dirent->d_namlen), "reclen:", dirent->d_reclen);
//     for (entry_offset = 0; (entry_offset < bytes_read) && (dirent->d_reclen > 0); entry_offset += dirent->d_reclen) {
//       dirent = reinterpret_cast<oi::FolderDirent*>(clone_buffer + entry_offset);

//       if (entry_cb) {
//         if (s64 cb_result = entry_cb(dirent); cb_result <= 0) return cb_result;
//       }
//     }
//     // Log("End position:", entry_offset, "/", clone_buffer_size);
//   } while (--_canary);

//   // Log("Dump ended with status", bytes_read);
//   sceKernelClose(fd_src);
//   return 0;
// }

// TEST(DirentTests, CompareDirentsAPP0) {
//   LogTest("<<<< Compare PFS read&getdirentries, clone to /data >>>>");

//   ///
//   /// Setup dumping directory
//   ///

//   std::vector<std::string> entries_read {};
//   std::vector<std::string> entries_getdirentries {};

//   sceKernelMkdir(clone_destination_read, 0777);
//   sceKernelMkdir(clone_destination_getdirentries, 0777);

//   //

//   LogTest("Read entries from /app0 (read) and clone to", clone_destination_read);
//   iterate_pfs_read(clone_source_app0, [&entries_read, dest = &clone_destination_read, cb = dump_clone_pfs_read](oi::PfsDirent* dirent) -> s64 {
//     if (is_directory_relatives(dirent->d_name)) return 0;
//     entries_read.emplace_back(std::string(dirent->d_name, dirent->d_namlen));
//     return cb(clone_destination_read, dirent);
//   });

//   LogTest("Read entries from /app0 (getdirentries) and clone to", clone_destination_getdirentries);
//   iterate_getdirentries(clone_source_app0,
//                         [&entries_getdirentries, dest = &clone_destination_getdirentries, cb = dump_clone_read](oi::FolderDirent* dirent) -> s64 {
//                           if (is_directory_relatives(dirent->d_name)) return 0;
//                           entries_getdirentries.emplace_back(std::string(dirent->d_name, dirent->d_namlen));
//                           return cb(clone_destination_getdirentries, dirent);
//                         });

//   std::sort(entries_getdirentries.begin(), entries_getdirentries.end());
//   std::sort(entries_read.begin(), entries_read.end());

//   CHECK_TRUE_TEXT(std::equal(entries_getdirentries.begin(), entries_getdirentries.end(), entries_read.begin(), entries_read.end()), "File lists are not
//   equal");
// }

// TEST(DirentTests, CompareDirentsData) {
//   LogTest("<<<< Compare regular read&getdirentries >>>>");

//   ///
//   /// Setup dumping directory
//   ///

//   std::vector<std::string> entries_read {};
//   std::vector<std::string> entries_getdirentries {};

//   //

//   LogTest("Read entries from /data (read)", clone_destination_read);
//   iterate_normal_read(clone_destination_read, [&entries_read](oi::FolderDirent* dirent) {
//     if (is_directory_relatives(dirent->d_name)) return 0;
//     entries_read.emplace_back(std::string(dirent->d_name, dirent->d_namlen));
//     return 0;
//   });

//   LogTest("Read entries from /data (getdirentries)", clone_destination_getdirentries);
//   iterate_getdirentries(clone_destination_getdirentries, [&entries_getdirentries](oi::FolderDirent* dirent) {
//     if (is_directory_relatives(dirent->d_name)) return 0;
//     entries_getdirentries.emplace_back(std::string(dirent->d_name, dirent->d_namlen));
//     return 0;
//   });

//   std::sort(entries_getdirentries.begin(), entries_getdirentries.end());
//   std::sort(entries_read.begin(), entries_read.end());

//   CHECK_TRUE_TEXT(std::equal(entries_getdirentries.begin(), entries_getdirentries.end(), entries_read.begin(), entries_read.end()), "File lists are not
//   equal");
// }

// OK
// TEST(DirentTests, DirentShortReadNormal) {
//   LogTest("<<<< Short normal reads >>>>");

//   const char* dir_test             = "/data/enderman/partialdirents";
//   char        file_placeholder[92] = "/data/enderman/partialdirents/tomakethingseasierthisfilenameshouldbeover40characterslong_aa";
//   char*       startchar            = file_placeholder + 90;
//   RegenerateDir(dir_test);

//   int           fd {};
//   s32           tbr {};
//   constexpr u32 buffer_size   = 4096;
//   u32           view_size     = 24;
//   u32           view_size_end = 24;
//   u32           zero          = 0;
//   char          buffer[buffer_size] {0};
//   char          reflection[buffer_size] {0};

//   auto quickprint = [&view_size, &view_size_end, bs = &buffer_size](std::string title, const void* array) -> void {
//     std::string out {title};
//     for (u32 idx = 0; idx < view_size; ++idx) {
//       if (!(idx % 12)) out += "\r\n\t\t\t\t\t\t";
//       auto hexed = to_hex<u16>(*(reinterpret_cast<const u8*>(array) + idx));
//       out += (hexed.length() == 1 ? "0" : "") + hexed + " ";
//     }
//     if (0 == view_size_end) {
//       Log(out);
//       return;
//     }
//     out += "\r\n\t\t\t\t\t\t----------";
//     for (u32 idx = 0; idx < view_size_end; ++idx) {
//       if (!(idx % 12)) out += "\r\n\t\t\t\t\t\t";
//       auto hexed = to_hex<u16>(*(reinterpret_cast<const u8*>(array) + *bs - view_size_end + idx));
//       out += (hexed.length() == 1 ? "0" : "") + hexed + " ";
//     }
//     Log(out);
//   };

//   auto rd = [](s32 fd, void* buffer, s64 size, s64 offset) -> s64 {
//     auto buffer_ptr = static_cast<char*>(buffer);
//     errno           = 0;
//     CHECK_EQUAL(offset, sceKernelLseek(fd, offset, 0));
//     CHECK_EQUAL_ZERO(errno);
//     memset(buffer_ptr, 0xAA, size);
//     errno = 0;
//     return sceKernelGetdirentries(fd, buffer_ptr, size, nullptr);
//   };

//   // clang-format off
//   errno = 0;  fd    = sceKernelOpen(dir_test, O_RDONLY, 0777);  CHECK_COMPARE(0, <, fd);

//   LogTest("EINVAL when size+offset falls before first 512 bytes");
//   for(u16 len=0; len < 512; len+=1){
//     tbr = rd(fd,buffer, len, 511-len);
//     if(tbr >= 0)
//       LogError("Incorrect returns for read size", len, "with offset",511-len);
//     CHECK_EQUAL(ORBIS_KERNEL_ERROR_EINVAL, tbr);
//     CHECK_EQUAL(EINVAL, errno); CHECK_TRUE(fillcheck(buffer, 0xAA, len));
//   }

//   view_size  = 24;  view_size_end = 24;
//   tbr = rd(fd, buffer, 512, 0);       CHECK_EQUAL(512, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R512", buffer);
//   tbr = rd(fd, reflection, 512, 5);   CHECK_EQUAL(507, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R512", reflection);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 5, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection, 512, 6);   CHECK_EQUAL(506, tbr);  CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R512", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 6, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection, 512,
//   7);   CHECK_EQUAL(505, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R512", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 7, reflection, tbr),
//   "memory compare failed");

//   view_size  = 24;  view_size_end = 24;
//   tbr = rd(fd, buffer, 513, 0);       CHECK_EQUAL(512, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R513", buffer);
//   tbr = rd(fd, reflection, 513, 5);   CHECK_EQUAL(507, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R513", reflection);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 5, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection, 513, 6);   CHECK_EQUAL(506, tbr);  CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R513", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 6, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection, 513,
//   7);   CHECK_EQUAL(505, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R513", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 7, reflection, tbr),
//   "memory compare failed");

//   // create some dummy files
//   for (*startchar = 'a'; *startchar < 'z'; *startchar += 1) {
//     CHECK_EQUAL_ZERO(touch(file_placeholder));
//   }

//   view_size  = 24;  view_size_end = 24;
//   // view_size  = 24;  view_size_end = 64;
//   // for this test, only "shorter" tbr is taken into account. dirents return data for x sectors only when end of the read happens on the next sector
//   // so 1023+0 is still 512, but 1023 + 5 offset is 507 + 512
//   tbr = rd(fd, buffer, 1023, 0);      CHECK_EQUAL(512, tbr);    CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R1023", buffer);
//   tbr = rd(fd, reflection, 1023, 5);  CHECK_EQUAL(1019, tbr);   CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R1023", reflection);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 5, reflection, 512 - 5), "memory compare failed"); tbr = rd(fd, reflection, 1023, 6);  CHECK_EQUAL(1018, tbr);   CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R1023", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 6, reflection, 512 - 6), "memory compare failed"); tbr = rd(fd, reflection,
//   1023, 7);  CHECK_EQUAL(1017, tbr);   CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R1023", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 7,
//   reflection, 512 - 7), "memory compare failed");

//   tbr = rd(fd, buffer, 1024, 0);      CHECK_EQUAL(1024, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R1024", buffer);
//   tbr = rd(fd, reflection, 1024, 5);  CHECK_EQUAL(1019, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R1024", reflection);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 5, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection, 1024, 6);  CHECK_EQUAL(1018, tbr);  CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R1024", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 6, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection,
//   1024, 7);  CHECK_EQUAL(1017, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R1024", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 7, reflection,
//   tbr), "memory compare failed");

//   tbr = rd(fd, buffer, 1025, 0);      CHECK_EQUAL(1024, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+0   R1025", buffer);
//   tbr = rd(fd, reflection, 1025, 5);  CHECK_EQUAL(1019, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+5   R1025", reflection);  CHECK_EQUAL_TEXT(-1,
//   imemcmp(buffer + 5, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection, 1025, 6);  CHECK_EQUAL(1018, tbr);  CHECK_EQUAL_ZERO(errno);
//   quickprint("\t+6   R1025", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 6, reflection, tbr), "memory compare failed"); tbr = rd(fd, reflection,
//   1025, 7);  CHECK_EQUAL(1017, tbr);  CHECK_EQUAL_ZERO(errno);  quickprint("\t+7   R1025", reflection);  CHECK_EQUAL_TEXT(-1, imemcmp(buffer + 7, reflection,
//   tbr), "memory compare failed");

//   // clang-format on

//   CHECK_EQUAL_ZERO(sceKernelClose(fd));
// }

// OK
// TEST(DirentTests, LseekRegularTests) {
//   LogTest("<<<< Testing lseek() on normal directory >>>>");

//   fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
//   CHECK_COMPARE_TEXT(fd, >, 0, "Unable to open /data/enderman");

//   int status;

//   // clang-format off
//   LogTest("START+0 == 0");
//   errno  = 0; status = sceKernelLseek(fd, 0, 0);
//   LONGLONGS_EQUAL_TEXT(0, status, "START+0"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("START-123 == EINVAL");
//   errno  = 0; status = sceKernelLseek(fd, -123, 0);
//   UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "START-123"); UNSIGNED_INT_EQUALS(EINVAL, errno);

//   LogTest("START+123456 == 123456");
//   errno  = 0; status = sceKernelLseek(fd, 123456, 0);
//   LONGLONGS_EQUAL_TEXT(123456, status, "START+123456"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("START+60 == 60");
//   errno  = 0; status = sceKernelLseek(fd, 60, 0);
//   LONGLONGS_EQUAL_TEXT(60, status, "START+60"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR+0 == 60");
//   errno  = 0; status = sceKernelLseek(fd, 0, 1);
//   LONGLONGS_EQUAL_TEXT(60, status, "CUR+0");  UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR+24 == 84");
//   errno  = 0; status = sceKernelLseek(fd, 24, 1);
//   LONGLONGS_EQUAL_TEXT(84, status, "CUR+24"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR-24 == 60");
//   errno  = 0; status = sceKernelLseek(fd, -24, 1);
//   LONGLONGS_EQUAL_TEXT(60, status, "CUR-24"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR-6666 == EINVAL");
//   errno  = 0; status = sceKernelLseek(fd, -6666, 1);
//   UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "CUR-6666");  UNSIGNED_INT_EQUALS(EINVAL, errno);

//   LogTest("CUR+123456 == 123516");
//   errno  = 0; status = sceKernelLseek(fd, 123456, 1);
//   LONGLONGS_EQUAL_TEXT(123516, status, "CUR+123456"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END+0 == 512");
//   errno  = 0; status = sceKernelLseek(fd, 0, 2);
//   LONGLONGS_EQUAL_TEXT(512, status, "END+0"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END+123456 == 123968");
//   errno  = 0; status = sceKernelLseek(fd, 123456, 2);
//   LONGLONGS_EQUAL_TEXT(123968, status, "END+123456"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END+100 == 612");
//   errno  = 0; status = sceKernelLseek(fd, 100, 2);
//   LONGLONGS_EQUAL_TEXT(612, status, "END+100"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END-100 == 412");
//   errno  = 0; status = sceKernelLseek(fd, -100, 2);
//   LONGLONGS_EQUAL_TEXT(412, status, "END-100"); UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END-100000 == EINVAL");
//   errno  = 0; status = sceKernelLseek(fd, -100000, 2);
//   UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");  UNSIGNED_INT_EQUALS(EINVAL, errno);
//   // clang-format off
// }

// TEST(DirentTests, LseekPFSTests) {
//   LogTest("Testing lseek() on PFS directory");

//   fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);
//   CHECK_COMPARE_TEXT(fd, >, 0, "Unable to open /app0/assets/misc");

//   s64 status;

//   // clang-format off
//   LogTest("START+0 == ");
//   errno  = 0; status = sceKernelLseek(fd, 0, 0);
//   LONGLONGS_EQUAL_TEXT(0, status, "START+0");                               UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("START-123 == EINVAL");
//   errno  = 0; status = sceKernelLseek(fd, -123, 0);
//   UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "START-123"); UNSIGNED_INT_EQUALS(EINVAL, errno);

//   LogTest("START+123456 == 123456");
//   errno  = 0; status = sceKernelLseek(fd, 123456, 0);
//   LONGLONGS_EQUAL_TEXT(123456, status, "START+123456");                     UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("START+60 == 60");
//   errno  = 0; status = sceKernelLseek(fd, 60, 0);
//   LONGLONGS_EQUAL_TEXT(60, status, "START+60");                             UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR+0 == 60");
//   errno  = 0; status = sceKernelLseek(fd, 0, 1);
//   LONGLONGS_EQUAL_TEXT(60, status, "CUR+0");                                UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR+24 == 84");
//   errno  = 0; status = sceKernelLseek(fd, 24, 1);
//   LONGLONGS_EQUAL_TEXT(84, status, "CUR+24");                               UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR-24 == 60");
//   errno  = 0; status = sceKernelLseek(fd, -24, 1);
//   LONGLONGS_EQUAL_TEXT(60, status, "CUR-24");                               UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("CUR-6666 == EINVAL");
//   errno  = 0; status = sceKernelLseek(fd, -6666, 1);
//   UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "CUR-6666");  UNSIGNED_INT_EQUALS(EINVAL, errno);

//   LogTest("CUR+123456 == 123516");
//   errno  = 0; status = sceKernelLseek(fd, 123456, 1);
//   LONGLONGS_EQUAL_TEXT(123516, status, "CUR+123456");                       UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END+0 == 65536");
//   errno  = 0; status = sceKernelLseek(fd, 0, 2);
//   LONGLONGS_EQUAL_TEXT(65536, status, "END+0");                             UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END+123456 == 188992");
//   errno  = 0; status = sceKernelLseek(fd, 123456, 2);
//   LONGLONGS_EQUAL_TEXT(188992, status, "END+123456");                       UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END+100 == 65636");
//   errno  = 0; status = sceKernelLseek(fd, 100, 2);
//   LONGLONGS_EQUAL_TEXT(65636, status, "END+100");                           UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END-100 == 65436");
//   errno  = 0; status = sceKernelLseek(fd, -100, 2);
//   LONGLONGS_EQUAL_TEXT(65436, status, "END-100");                           UNSIGNED_INT_EQUALS(0, errno);

//   LogTest("END-100000 == EINVAL");
//   errno  = 0; status = sceKernelLseek(fd, -100000, 2);
//   UNSIGNED_INT_EQUALS_TEXT(ORBIS_KERNEL_ERROR_EINVAL, status, "END-100000");  UNSIGNED_INT_EQUALS(EINVAL, errno);
//   // clang-format on
// }