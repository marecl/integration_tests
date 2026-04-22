#include "fs_test.h"

#include "calculator.h"
#include "fs_test_constants.h"
#include "orbis/UserService.h"

#include <CppUTest/TestHarness.h>
#include <chrono>
#include <cstdlib>
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

std::string is_einval_str(s64 value) {
  constexpr int _einval     = ORBIS_KERNEL_ERROR_EINVAL;
  constexpr s64 einval_cast = s64(_einval);
  return value == einval_cast ? "EINVAL" : std::to_string(value);
}

s64 compare_data_dump(const void* master, const void* test, s64 buffer_size, s64 tbr, s64 offset);

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

  s64 entry_counter {0};
  for (auto& dent: fs::directory_iterator("/app0/assets/misc")) {
    target = clone_dir / dent.path().filename();
    if (dent.is_regular_file()) {
      status = touch(target.c_str());
      entry_counter++;
      continue;
    }
    if (dent.is_directory()) {
      status = sceKernelMkdir(target.c_str(), 0777);
      entry_counter++;
      continue;
    }
    LogError("Can't create", target.string());
    return false;
  }

  Log("Cloned into", entry_counter, "elements");
  return true;
}

s64 undump_file(const char* path, char* data, u64 length) {
  int fd = sceKernelOpen(path, O_RDONLY, 0777);
  if (fd < 0) return fd;
  memset(data, 0xAA, length);
  int tbr = sceKernelRead(fd, data, length);
  if (tbr < 0) return tbr;
  if (auto res = sceKernelClose(fd); res < 0) return res;
  return tbr;
}

const char* input_pfs                   = "/app0/assets/misc";
const char* input_normal                = "/data/enderman/clone";
const char* output_dir                  = "/data/enderman/dump";
const char* output_pfs_read             = "/data/enderman/dump/pfs_read.bin";
const char* output_pfs_getdirentries    = "/data/enderman/dump/pfs_getdirent.bin";
const char* output_normal_read          = "/data/enderman/dump/normal_read.bin";
const char* output_normal_getdirentries = "/data/enderman/dump/normal_getdirent.bin";

static std::vector<int> open_fd {};

static void add_fd(int fd) {
  open_fd.emplace_back(fd);
}

TEST_GROUP (DirentTests) {

  void setup() {
    open_fd.clear();
  }

  void teardown() {
    for (const auto fd: open_fd) {
      if (sceKernelClose(fd) < 0) continue;
      LogWarning("Closed leftover fd:", fd);
    }
    open_fd.clear();
  }
};

TEST(DirentTests, PFSGetdirentriesFuzz) {
  LogTest("<<<< PFS getdirentries fuzzing test >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(pfs_getdirentries_target, master_length);
  LogTest("Master PFS getdirentries length is", master_length);

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = rand() % master_length * 2 - master_length;
    s64 spec_size   = rand() % master_length * 2 - master_length;

    memset(buffer, 0xAA, 65536);
    calculate_pfs_getdirentries(&calc, master_buffer, master_length, spec_offset, spec_size);

    fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    add_fd(fd);
    CHECK_EQUAL(calc.expected_lseek, sceKernelLseek(fd, calc.read_offset, 0));
    errno = 0;
    tbr   = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    // if (tbr >= 0)
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    LogTest(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", basep,
            is_einval_str(tbr), end_ptr_position, to_hex_string(buffer, 16, ""));
    // is_einval_str(tbr), (tbr >= 0) ? std::to_string(end_ptr_position) : "[unavailable]", to_hex_string(buffer, 16, ""));

    compare_data_dump(master_buffer, buffer, 65536, tbr, calc.meta_dirent_start);
    CHECK_EQUAL_TEXT(calc.expected_basep, basep, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, errno, "Bad errno");
  }
}

TEST(DirentTests, NormalGetdirentriesFuzz) {
  LogTest("<<<< Normal getdirentries fuzzing test >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_normal_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(normal_getdirentries_target, master_length);
  LogTest("Master Normal getdirentries length is", master_length);

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = rand() % master_length * 2 - master_length;
    s64 spec_size   = rand() % master_length * 2 - master_length;

    memset(buffer, 0xAA, 65536);
    calculate_normal_getdirentries(&calc, master_buffer, master_length, spec_offset, spec_size);

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    add_fd(fd);
    CHECK_EQUAL(calc.expected_lseek, sceKernelLseek(fd, calc.read_offset, 0));
    errno = 0;
    tbr   = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    // if (tbr >= 0)
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    LogTest(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", basep,
            is_einval_str(tbr), end_ptr_position, to_hex_string(buffer, 16, ""));
    // is_einval_str(tbr), (tbr >= 0) ? std::to_string(end_ptr_position) : "[unavailable]", to_hex_string(buffer, 16, ""));

    compare_data_dump(master_buffer, buffer, 65536, tbr, calc.meta_dirent_start);
    CHECK_EQUAL(calc.expected_basep, basep);
    CHECK_EQUAL(calc.expected_result, tbr);
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Incorrect pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, errno, "Incorrect errno");
  }
}

TEST(DirentTests, NormalReadFuzz) {
  LogTest("<<<< Normal read fuzzing test >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_normal_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   real_start {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(normal_read_target, master_length);
  LogTest("Master Normal read length is", master_length, ",", "testing", FUZZ_MAX_ITERATIONS, "samples", ",", "max", FUZZ_MAX_FAILURES, "failures allowed");

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = rand() % master_length * 2 - master_length;
    s64 spec_size   = rand() % master_length * 2 - master_length;

    memset(buffer, 0xAA, 65536);
    calculate_pfs_read(&calc, master_buffer, master_length, spec_offset, spec_size);

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    add_fd(fd);
    if (real_start = sceKernelLseek(fd, calc.read_offset, 0); real_start != calc.expected_lseek) {
      LogError("lseek() expected:", calc.expected_lseek, ",", "return:", real_start);
      // FAIL("Incorrect lseek() return");
      ++failed_samples;
      continue;
    }
    errno            = 0;
    tbr              = sceKernelRead(fd, buffer, calc.read_size);
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    s64 diff_idx = compare_data_dump(master_buffer, buffer, 65536, tbr, calc.expected_basep);

    if (diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_result != tbr || calc.expected_end_position != end_ptr_position || calc.expected_errno != errno || diff_idx <= 0) {
      ++failed_samples;
      LogError(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", real_start,
               is_einval_str(tbr), end_ptr_position);
    }
  }

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, PFSReadFuzz) {
  LogTest("<<<< PFS read fuzzing test >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   real_start {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(pfs_read_target, master_length);
  LogTest("Master PFS read length is", master_length, ",", "testing", FUZZ_MAX_ITERATIONS, "samples", ",", "max", FUZZ_MAX_FAILURES, "failures allowed");

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = rand() % master_length * 2 - master_length;
    s64 spec_size   = rand() % master_length * 2 - master_length;

    memset(buffer, 0xAA, 65536);
    calculate_pfs_read(&calc, master_buffer, master_length, spec_offset, spec_size);

    fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    add_fd(fd);
    if (real_start = sceKernelLseek(fd, calc.read_offset, 0); real_start != calc.expected_lseek) {
      LogError("lseek() expected:", calc.expected_lseek, ",", "return:", real_start);
      // FAIL("Incorrect lseek() return");
      ++failed_samples;
      continue;
    }
    errno            = 0;
    tbr              = sceKernelRead(fd, buffer, calc.read_size);
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    s64 diff_idx = compare_data_dump(master_buffer, buffer, 65536, tbr, calc.expected_basep);

    if (diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_result != tbr || calc.expected_end_position != end_ptr_position || calc.expected_errno != errno || diff_idx <= 0) {
      ++failed_samples;
      LogError(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", real_start,
               is_einval_str(tbr), end_ptr_position);
    }
  }

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, PFSGetdirentries) {
  LogTest("<<<< PFS getdirentries tests >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(pfs_getdirentries_target, master_length);
  LogTest("Master PFS getdirentries length is", master_length);

  for (const auto& spec: pfs_dirent_variants) {
    memset(buffer, 0xAA, 65536);
    calculate_pfs_getdirentries(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    add_fd(fd);
    CHECK_EQUAL(calc.expected_lseek, sceKernelLseek(fd, calc.read_offset, 0));
    errno = 0;
    tbr   = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    // if (tbr >= 0)
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    LogTest(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", basep,
            is_einval_str(tbr), end_ptr_position, to_hex_string(buffer, 16, ""));
    // is_einval_str(tbr), (tbr >= 0) ? std::to_string(end_ptr_position) : "[unavailable]", to_hex_string(buffer, 16, ""));

    compare_data_dump(master_buffer, buffer, 65536, tbr, calc.meta_dirent_start);
    CHECK_EQUAL_TEXT(calc.expected_basep, basep, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, errno, "Bad errno");
  }
}

TEST(DirentTests, NormalGetdirentries) {
  LogTest("<<<< Normal getdirentries tests >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_normal_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(normal_getdirentries_target, master_length);
  LogTest("Master Normal getdirentries length is", master_length);

  for (const auto& spec: normal_dirent_variants) {
    memset(buffer, 0xAA, 65536);
    calculate_normal_getdirentries(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    add_fd(fd);
    CHECK_EQUAL(calc.expected_lseek, sceKernelLseek(fd, calc.read_offset, 0));
    errno = 0;
    tbr   = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    // if (tbr >= 0)
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    LogTest(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", basep,
            is_einval_str(tbr), end_ptr_position, to_hex_string(buffer, 16, ""));
    // is_einval_str(tbr), (tbr >= 0) ? std::to_string(end_ptr_position) : "[unavailable]", to_hex_string(buffer, 16, ""));

    compare_data_dump(master_buffer, buffer, 65536, tbr, calc.meta_dirent_start);
    CHECK_EQUAL(calc.expected_basep, basep);
    CHECK_EQUAL(calc.expected_result, tbr);
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Incorrect pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, errno, "Incorrect errno");
  }
}

TEST(DirentTests, PFSRead) {
  LogTest("<<<< PFS read tests >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   real_start {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(pfs_read_target, master_length);
  LogTest("Master PFS read length is", master_length);

  for (const auto& spec: pfs_read_variants) {
    memset(buffer, 0xAA, 65536);
    calculate_pfs_read(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    add_fd(fd);
    CHECK_EQUAL(calc.expected_lseek, sceKernelLseek(fd, calc.read_offset, 0));
    real_start = sceKernelLseek(fd, 0, 1);
    errno      = 0;
    tbr        = sceKernelRead(fd, buffer, calc.read_size);
    // if (tbr >= 0)
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    LogTest(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", real_start,
            is_einval_str(tbr), end_ptr_position, "\t", to_hex_string(buffer, 16, ""));
    // is_einval_str(tbr), (tbr >= 0) ? std::to_string(end_ptr_position) : "[unavailable]", "\t", to_hex_string(buffer, 16, ""));

    compare_data_dump(master_buffer, buffer, 65536, tbr, calc.expected_basep);
    CHECK_EQUAL_TEXT(calc.expected_basep, real_start, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, errno, "Bad errno");
  }
}

TEST(DirentTests, NormalRead) {
  LogTest("<<<< Normal read tests >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_normal_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   real_start {};
  s64                   end_ptr_position {};

  CHECK_EQUAL(normal_read_target, master_length);
  LogTest("Master Normal read length is", master_length);

  for (const auto& spec: normal_read_variants) {
    memset(buffer, 0xAA, 65536);
    calculate_normal_read(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    add_fd(fd);
    CHECK_EQUAL(calc.expected_lseek, sceKernelLseek(fd, calc.read_offset, 0));
    real_start = sceKernelLseek(fd, 0, 1);
    errno      = 0;
    tbr        = sceKernelRead(fd, buffer, calc.read_size);
    // if (tbr >= 0)
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    LogTest(calc.read_size, calc.read_offset, calc.expected_basep, is_einval_str(calc.expected_result), calc.expected_end_position, "\t->\t", real_start,
            is_einval_str(tbr), end_ptr_position, "\t", to_hex_string(buffer, 16, ""));
    // is_einval_str(tbr), (tbr >= 0) ? std::to_string(end_ptr_position) : "[unavailable]", "\t", to_hex_string(buffer, 16, ""));

    compare_data_dump(master_buffer, buffer, 65536, tbr, calc.expected_basep);
    CHECK_EQUAL_TEXT(calc.expected_basep, real_start, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Incorrect read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Incorrect pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, errno, "Incorrect errno");
  }
}

TEST(DirentTests, ValidateDirentries) {
  LogTest("<<<< Dirent structure validation >>>>");
  char buffer[65536];
  s64  master_length {};
  s64  offset {};

  // these tests are not done yet
  LogTest("Normal read");
  master_length = undump_file(output_normal_read, buffer, 65536);
  CHECK_EQUAL(normal_read_target, validate_normal_getdirentries(buffer, master_length));
  LogTest("Normal getdirentries");
  master_length = undump_file(output_normal_getdirentries, buffer, 65536);
  CHECK_EQUAL(normal_getdirentries_target, validate_normal_getdirentries(buffer, master_length));

  LogTest("PFS read");
  master_length = undump_file(output_pfs_read, buffer, 65536);
  CHECK_EQUAL(pfs_read_target, validate_pfs_read(buffer, master_length));
  LogTest("PFS getdirentries");
  master_length = undump_file(output_pfs_getdirentries, buffer, 65536);
  CHECK_EQUAL(pfs_getdirentries_target, validate_pfs_getdirentries(buffer, master_length));
}

s64 NormalComparator(const char* read, const char* getdirentries, u64 length) {
  s64 offset {0};
  u64 entry_counter {0};
  while (offset < length) {
    const oi::FolderDirent* dirent_read          = reinterpret_cast<const oi::FolderDirent*>(read + offset);
    const oi::FolderDirent* dirent_getdirentries = reinterpret_cast<const oi::FolderDirent*>(getdirentries + offset);

    if (dirent_read->d_namlen != dirent_getdirentries->d_namlen) break;
    if (dirent_read->d_reclen != dirent_getdirentries->d_reclen) break;
    if (dirent_read->d_type != dirent_getdirentries->d_type) break;
    if (memcmp(dirent_read->d_name, dirent_getdirentries->d_name, dirent_read->d_namlen)) break; // namlen is the same
    if (offset + dirent_read->d_reclen > length) break;
    offset += dirent_read->d_reclen;
    if (dirent_read->d_reclen == 0) break;
    entry_counter++;
  }

  Log("Compared", entry_counter, "entries");
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
  u64 entry_counter {0};
  while (offset < length) {
    const oi::PfsDirent*    dirent_read          = reinterpret_cast<const oi::PfsDirent*>(read + offset);
    const oi::FolderDirent* dirent_getdirentries = reinterpret_cast<const oi::FolderDirent*>(getdirentries + offset);

    if (dirent_read->d_namlen != dirent_getdirentries->d_namlen) break;
    if (dirent_read->d_reclen != dirent_getdirentries->d_reclen) break;
    // if (dirent_read->d_type != dirent_getdirentries->d_type) break;
    if (memcmp(dirent_read->d_name, dirent_getdirentries->d_name, dirent_read->d_namlen)) break; // namlen is the same
    if (dirent_read->d_reclen == 0) break;                                                       // reclen is the same
    if (offset + dirent_read->d_reclen > length) break;
    offset += dirent_read->d_reclen;
  }

  Log("Compared", entry_counter, "entries");
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
  s64  tbr {};

  RegenerateDir(output_dir);

  //

  fd_read = sceKernelOpen(input_pfs, O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd_read, >, 0, "Can't open input dir");
  add_fd(fd_read);

  //

  fd_dump = sceKernelOpen(output_pfs_read, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelRead(fd_read, buffer, 65536);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "PFS read failed");
    LogTest("PFS read got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(pfs_read_file_size_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  fd_dump = sceKernelOpen(output_pfs_getdirentries, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  CHECK_EQUAL_ZERO_TEXT(sceKernelLseek(fd_read, 0, 0), "Can't rewind directory");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelGetdirentries(fd_read, buffer, 65536, nullptr);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "PFS sceKernelGetdirentries failed");
    LogTest("PFS sceKernelGetdirentries got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(pfs_getdirentries_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  CHECK_EQUAL_ZERO(sceKernelClose(fd_read));
  fd_read = sceKernelOpen(input_normal, O_DIRECTORY | O_RDONLY, 0777);
  add_fd(fd_read);
  CHECK_COMPARE_TEXT(fd_read, >, 0, "Can't open input dir");

  //

  fd_dump = sceKernelOpen(output_normal_read, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelRead(fd_read, buffer, 65536);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "Normal read failed");
    LogTest("Normal read got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(normal_read_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //
  fd_dump = sceKernelOpen(output_normal_getdirentries, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  CHECK_EQUAL_ZERO_TEXT(sceKernelLseek(fd_read, 0, 0), "Can't rewind directory");
  do {
    memset(buffer, 0xAA, 65536);
    tbr = sceKernelGetdirentries(fd_read, buffer, 65536, nullptr);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "Normal sceKernelGetdirentries failed");
    LogTest("Normal sceKernelGetdirentries got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL_TEXT(normal_getdirentries_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  CHECK_EQUAL_ZERO(sceKernelClose(fd_read));
}

// 1 on equal, <=0 on diff idx
s64 compare_data_dump(const void* master, const void* test, s64 buffer_size, s64 tbr, s64 offset) {
  if (tbr == 0) return 1;
  const char* master_data = reinterpret_cast<const char*>(master) + offset;
  const char* test_data   = reinterpret_cast<const char*>(test);

  if (s64 idx = imemcmp(master_data, test_data, tbr); idx <= 0) {
    // differing idx is negative
    return idx;
  }
  return 1;
}

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