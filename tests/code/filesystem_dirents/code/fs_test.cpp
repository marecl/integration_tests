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

bool PrepareTests() {
  fs::path clone_dir = "/data/enderman/clone";
  fs::path target {};
  s32      status {};

  RegenerateDir("/data/enderman");
  sceKernelMkdir(clone_dir.c_str(), 0777);

  s64 entry_counter {0};
  // ironic because directory_iterator uses dirents hehe
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
  if (entry_counter != files_cloned_target) LogError("Not all files cloned:", entry_counter, "out of", files_cloned_target);
  return true;
}

s64 undump_file(const char* path, char* data, u64 length) {
  int fd = sceKernelOpen(path, O_RDONLY, 0777);
  if (fd < 0) return fd;
  memset(data, DEFAULT_8, length);
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

TEST_GROUP (DirentTests) {
  std::vector<int> open_fd {};

  void add_fd(int fd) {
    open_fd.emplace_back(fd);
  }

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
  LogTest("Note: Only first 20 bad entries are shown");
  LogTest("Note: This may take a while");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   basep_canary {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(pfs_getdirentries_target, master_length);

  LogTest("Master PFS getdirentries length is", master_length, ",", "testing", FUZZ_MAX_ITERATIONS, "samples", ",", "max", FUZZ_MAX_FAILURES,
          "bad returns are shown");

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};

  fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
  add_fd(fd);

  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = get_fuzz();
    s64 spec_size   = rand() % (65536 + 1);

    memset(buffer, DEFAULT_8, 65536);
    basep        = DEFAULT_64;
    basep_canary = basep;

    calculate_pfs_getdirentries(&calc, master_buffer, master_length, spec_offset, spec_size);
    CHECK_COMPARE_TEXT(65536, >=, calc.expected_result, "Calculation attempted to overflow the buffer");

    sceKernelLseek(fd, calc.read_offset, 0);
    errno            = 0;
    tbr              = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);

    // -1 means no change
    if (calc.expected_basep == -1) calc.expected_basep = basep_canary;

    if (calc.expected_basep != basep || calc.expected_result != tbr || calc.expected_end_position != end_ptr_position ||
        calc.expected_errno != hardware_errno) {
      if (++failed_samples >= FUZZ_MAX_FAILURES) continue;
      LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
               "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
    }

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }
  }

  sceKernelClose(fd);

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, NormalGetdirentriesFuzz) {
  LogTest("<<<< Normal getdirentries fuzzing test >>>>");
  LogTest("Note: Only first 20 bad entries are shown");
  LogTest("Note: This may take a while");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_normal_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   basep_canary {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(normal_getdirentries_target, master_length);

  LogTest("Master Normal getdirentries length is", master_length, ",", "testing", FUZZ_MAX_ITERATIONS, "samples", ",", "max", FUZZ_MAX_FAILURES,
          "bad returns are shown");

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = get_fuzz();
    s64 spec_size   = rand() % (65536 + 1);

    memset(buffer, DEFAULT_8, 65536);
    basep        = DEFAULT_64;
    basep_canary = basep;

    calculate_normal_getdirentries(&calc, master_buffer, master_length, spec_offset, spec_size);
    CHECK_COMPARE_TEXT(65536, >=, calc.expected_result, "Calculation attempted to overflow the buffer");

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    sceKernelLseek(fd, calc.read_offset, 0);
    errno            = 0;
    tbr              = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    // -1 means no change
    if (calc.expected_basep == -1) calc.expected_basep = basep_canary;

    if (calc.expected_basep != basep || calc.expected_result != tbr || calc.expected_end_position != end_ptr_position ||
        calc.expected_errno != hardware_errno) {
      if (++failed_samples >= FUZZ_MAX_FAILURES) continue;
      LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
               "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
    }

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
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
  // -1 basep means "no change"
  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_getdirentries, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   basep_canary {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(pfs_getdirentries_target, master_length);

  LogTest("Master PFS getdirentries length is", master_length);

  for (const auto& spec: pfs_dirent_variants) {
    memset(buffer, DEFAULT_8, 65536);
    basep        = DEFAULT_64;
    basep_canary = basep;

    calculate_pfs_getdirentries(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    sceKernelLseek(fd, calc.read_offset, 0);
    errno            = 0;
    tbr              = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    // LogTest(calc.read_size, calc.read_offset, "->", calc.expected_basep, "(", calc.meta_dirent_start, ")", val_or_err(calc.expected_result),
    //         calc.expected_end_position, calc.expected_errno, "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_basep == -1) calc.expected_basep = basep_canary;
    if (calc.expected_basep == basep && calc.expected_result == tbr && calc.expected_end_position == end_ptr_position && calc.expected_errno == hardware_errno)
      continue;

    LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
             "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
    CHECK_EQUAL_TEXT(calc.expected_basep == -1 ? basep_canary : calc.expected_basep, basep, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, hardware_errno, "Bad errno");
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
  s64                   basep_canary {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(normal_getdirentries_target, master_length);

  LogTest("Master Normal getdirentries length is", master_length);

  for (const auto& spec: normal_dirent_variants) {
    memset(buffer, DEFAULT_8, 65536);
    basep        = DEFAULT_64;
    basep_canary = basep;

    calculate_normal_getdirentries(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    sceKernelLseek(fd, calc.read_offset, 0);
    errno            = 0;
    tbr              = sceKernelGetdirentries(fd, buffer, calc.read_size, &basep);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    // LogTest(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
    //         "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_basep == -1) calc.expected_basep = basep_canary;
    if (calc.expected_basep == basep && calc.expected_result == tbr && calc.expected_end_position == end_ptr_position && calc.expected_errno == hardware_errno)
      continue;

    LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
             "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
    CHECK_EQUAL_TEXT(calc.expected_basep == -1 ? basep_canary : calc.expected_basep, basep, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, hardware_errno, "Bad errno");
  }
}

TEST(DirentTests, PFSReadFuzz) {
  LogTest("<<<< PFS read fuzzing test >>>>");
  LogTest("Note: Only first 20 bad entries are shown");
  LogTest("Note: This may take a while");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(pfs_read_file_size_target, master_length);

  LogTest("Master PFS read length is", master_length, ",", "testing", FUZZ_MAX_ITERATIONS, "samples", ",", "max", FUZZ_MAX_FAILURES, "bad returns are shown");

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = get_fuzz();
    s64 spec_size   = rand() % (65536 + 1);

    memset(buffer, DEFAULT_8, 65536);
    calculate_pfs_read(&calc, master_buffer, master_length, spec_offset, spec_size);

    fd               = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    basep            = sceKernelLseek(fd, calc.read_offset, 0);
    errno            = 0;
    tbr              = sceKernelRead(fd, buffer, calc.read_size);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_basep == basep && calc.expected_result == tbr && calc.expected_end_position == end_ptr_position && calc.expected_errno == hardware_errno)
      continue;
    if (++failed_samples >= FUZZ_MAX_FAILURES) continue;
    LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
             "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
  }

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, NormalReadFuzz) {
  LogTest("<<<< Normal read fuzzing test >>>>");
  LogTest("Note: Only first 20 bad entries are shown");
  LogTest("Note: This may take a while");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_normal_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(normal_read_target, master_length);

  LogTest("Master Normal read length is", master_length, ",", "testing", FUZZ_MAX_ITERATIONS, "samples", ",", "max", FUZZ_MAX_FAILURES,
          "bad returns are shown");

  srand(time(nullptr));

  s64 sample_num {};
  s64 failed_samples {};
  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = get_fuzz();
    s64 spec_size   = rand() % (65536 + 1);

    memset(buffer, DEFAULT_8, 65536);
    calculate_pfs_read(&calc, master_buffer, master_length, spec_offset, spec_size);

    fd               = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    basep            = sceKernelLseek(fd, calc.read_offset, 0);
    errno            = 0;
    tbr              = sceKernelRead(fd, buffer, calc.read_size);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_basep == basep && calc.expected_result == tbr && calc.expected_end_position == end_ptr_position && calc.expected_errno == hardware_errno)
      continue;
    if (++failed_samples >= FUZZ_MAX_FAILURES) continue;
    LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
             "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
  }

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, PFSRead) {
  LogTest("<<<< PFS read tests >>>>");

  int                   fd {};
  s64                   tbr {};
  char                  buffer[65536];
  char                  master_buffer[65536];
  const s64             master_length = undump_file(output_pfs_read, master_buffer, 65536);
  oi::DirentCombination calc {};
  s64                   basep {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(pfs_read_file_size_target, master_length);

  LogTest("Master PFS read length is", master_length);

  for (const auto& spec: pfs_read_variants) {
    memset(buffer, DEFAULT_8, 65536);
    calculate_pfs_read(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
    sceKernelLseek(fd, calc.read_offset, 0);
    basep            = sceKernelLseek(fd, 0, 1);
    errno            = 0;
    tbr              = sceKernelRead(fd, buffer, calc.read_size);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    // LogTest(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
    //         "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_basep == basep && calc.expected_result == tbr && calc.expected_end_position == end_ptr_position && calc.expected_errno == hardware_errno)
      continue;

    LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
             "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
    CHECK_EQUAL_TEXT(calc.expected_basep, basep, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, hardware_errno, "Bad errno");
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
  s64                   basep {};
  s64                   end_ptr_position {};
  int                   hardware_errno {};

  CHECK_EQUAL(normal_read_target, master_length);

  LogTest("Master Normal read length is", master_length);

  for (const auto& spec: normal_read_variants) {
    memset(buffer, DEFAULT_8, 65536);
    calculate_normal_read(&calc, master_buffer, master_length, spec.offset, spec.size);

    fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
    sceKernelLseek(fd, calc.read_offset, 0);
    basep            = sceKernelLseek(fd, 0, 1);
    errno            = 0;
    tbr              = sceKernelRead(fd, buffer, calc.read_size);
    hardware_errno   = errno;
    end_ptr_position = sceKernelLseek(fd, 0, 1);
    sceKernelClose(fd);

    // LogTest(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
    //         "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));

    if (s64 diff_idx = compare_data_dump(master_buffer, buffer, master_length, tbr, calc.meta_dirent_start); diff_idx <= 0) {
      LogError("Inconsistent read at/idx", calc.read_offset - diff_idx, -diff_idx);
      LogError("Global dump:", to_hex_string(master_buffer - diff_idx, 32, ""));
      LogError("Recent dump:", to_hex_string(buffer - diff_idx, 32, ""));
    }

    if (calc.expected_basep == basep && calc.expected_result == tbr && calc.expected_end_position == end_ptr_position && calc.expected_errno == hardware_errno)
      continue;

    LogError(calc.read_size, calc.read_offset, "->", calc.expected_basep, val_or_err(calc.expected_result), calc.expected_end_position, calc.expected_errno,
             "->", basep, val_or_err(tbr), end_ptr_position, hardware_errno, to_hex_string(buffer, 16, ""));
    CHECK_EQUAL_TEXT(calc.expected_basep, basep, "Bad starting position");
    CHECK_EQUAL_TEXT(calc.expected_result, tbr, "Bad read size");
    CHECK_EQUAL_TEXT(calc.expected_end_position, end_ptr_position, "Bad pointer position after read");
    CHECK_EQUAL_TEXT(calc.expected_errno, hardware_errno, "Bad errno");
  }
}

TEST(DirentTests, PFSLSeekFuzz) {
  LogTest("<<<< PFS lseek fuzzing test >>>>");
  LogTest("Note: Only first 20 bad entries are shown");

  int fd {};
  s64 master_length = pfs_read_file_size_target; // pfs has 64k blocks
  s64 current_offset {};
  s64 previous_offset {};
  s64 expected_offset {};
  s64 hardware_offset {};
  int expected_errno {};
  int hardware_errno {};

  s64 sample_num {};
  s64 failed_samples {};
  fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
  add_fd(fd);

  CHECK_EQUAL(master_length, sceKernelLseek(fd, 0, 2));
  CHECK_EQUAL_ZERO(sceKernelLseek(fd, 0, 0));
  current_offset = 0;

  srand(time(nullptr));

  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = get_fuzz();
    int spec_whence = rand() % 6 - 1; // -1 to 4; // -1 to 4

    expected_offset = calculate_lseek(master_length, current_offset, spec_offset, spec_whence, &expected_errno);
    errno           = 0;
    hardware_offset = sceKernelLseek(fd, spec_offset, spec_whence);
    hardware_errno  = errno;

    previous_offset = current_offset;
    if (hardware_offset >= 0) current_offset = hardware_offset; // set this because if one fails, all subsequent ones fail too
    if (expected_offset == hardware_offset && expected_errno == hardware_errno) {
      continue;
    }

    if (++failed_samples >= FUZZ_MAX_FAILURES) continue;

    LogError("Size:", master_length, "Spec off:", spec_offset, "Spec whence:", spec_whence, "Start off:", previous_offset, "Expected off:", expected_offset,
             "DUT off:", hardware_offset, "Expected errno:", expected_errno, "DUT errno:", hardware_errno);
  }

  sceKernelClose(fd);

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, NormalLSeekFuzz) {
  LogTest("<<<< Normal lseek fuzzing test >>>>");
  LogTest("Note: Only first 20 bad entries are shown");

  int fd {};
  s64 master_length = normal_read_target;
  s64 current_offset {};
  s64 previous_offset {};
  s64 expected_offset {};
  s64 hardware_offset {};
  int expected_errno {};
  int hardware_errno {};

  s64 sample_num {};
  s64 failed_samples {};

  fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
  add_fd(fd);

  CHECK_EQUAL(master_length, sceKernelLseek(fd, 0, 2));
  CHECK_EQUAL_ZERO(sceKernelLseek(fd, 0, 0));
  current_offset = 0;

  srand(time(nullptr));

  for (sample_num = 0; sample_num < FUZZ_MAX_ITERATIONS; sample_num++) {
    s64 spec_offset = get_fuzz();
    int spec_whence = rand() % 6 - 1; // -1 to 4; // -1 to 4

    expected_offset = calculate_lseek(master_length, current_offset, spec_offset, spec_whence, &expected_errno);
    errno           = 0;
    hardware_offset = sceKernelLseek(fd, spec_offset, spec_whence);
    hardware_errno  = errno;

    previous_offset = current_offset;
    if (hardware_offset >= 0) current_offset = hardware_offset; // set this because if one fails, all subsequent ones fail too
    if (expected_offset == hardware_offset && expected_errno == hardware_errno) {
      continue;
    }

    if (++failed_samples >= FUZZ_MAX_FAILURES) continue;

    LogError("Size:", master_length, "Spec off:", spec_offset, "Spec whence:", spec_whence, "Start off:", previous_offset, "Expected off:", expected_offset,
             "DUT off:", hardware_offset, "Expected errno:", expected_errno, "DUT errno:", hardware_errno);
  }

  sceKernelClose(fd);

  double fail_rate = 100.0f * failed_samples / sample_num;
  if (fail_rate == 0.0f) {
    LogSuccess("Fuzzing passed!");
    return;
  }
  Log("Failure rate%:", fail_rate, "(", failed_samples, "/", sample_num, "failed )");
}

TEST(DirentTests, PFSLSeekTests) {
  LogTest("<<<< PFS lseek tests >>>>");

  int fd {};
  s64 master_length = pfs_read_file_size_target;
  s64 current_offset {};
  s64 previous_offset {};
  s64 expected_offset {};
  s64 hardware_offset {};
  int expected_errno {};
  int hardware_errno {};

  fd = sceKernelOpen(input_pfs, O_DIRECTORY, 0777);
  add_fd(fd);

  CHECK_EQUAL(master_length, sceKernelLseek(fd, 0, 2));
  CHECK_EQUAL_ZERO(sceKernelLseek(fd, 0, 0));
  current_offset = 0;

  for (const auto& spec: pfs_lseek_variants) {
    expected_offset = calculate_lseek(master_length, current_offset, spec.offset, spec.whence, &expected_errno);
    errno           = 0;
    hardware_offset = sceKernelLseek(fd, spec.offset, spec.whence);
    hardware_errno  = errno;

    // LogTest(previous_offset, spec.offset, spec.whence, "->", val_or_err(expected_offset), expected_errno, "->", val_or_err(hardware_offset), hardware_errno);

    previous_offset = current_offset;
    if (hardware_offset >= 0) current_offset = hardware_offset; // set this because if one fails, all subsequent ones fail too
    if (expected_offset == hardware_offset && expected_errno == hardware_errno) {
      continue;
    }

    LogError("Size:", master_length, "Spec off:", spec.offset, "Spec whence:", spec.whence, "Start off:", previous_offset, "Expected off:", expected_offset,
             "DUT off:", hardware_offset, "Expected errno:", expected_errno, "DUT errno:", hardware_errno);
  }

  sceKernelClose(fd);
}

TEST(DirentTests, NormalLSeekTests) {
  LogTest("<<<< Normal lseek tests >>>>");

  int fd {};
  s64 master_length = normal_read_target;
  s64 current_offset {};
  s64 previous_offset {};
  s64 expected_offset {};
  s64 hardware_offset {};
  int expected_errno {};
  int hardware_errno {};

  fd = sceKernelOpen(input_normal, O_DIRECTORY, 0777);
  add_fd(fd);

  CHECK_EQUAL(master_length, sceKernelLseek(fd, 0, 2));
  CHECK_EQUAL_ZERO(sceKernelLseek(fd, 0, 0));
  current_offset = 0;

  for (const auto& spec: normal_lseek_variants) {
    expected_offset = calculate_lseek(master_length, current_offset, spec.offset, spec.whence, &expected_errno);
    errno           = 0;
    hardware_offset = sceKernelLseek(fd, spec.offset, spec.whence);
    hardware_errno  = errno;

    // LogTest(previous_offset, spec.offset, spec.whence, "->", val_or_err(expected_offset), expected_errno, "->", val_or_err(hardware_offset), hardware_errno);

    previous_offset = current_offset;
    if (hardware_offset >= 0) current_offset = hardware_offset; // set this because if one fails, all subsequent ones fail too
    if (expected_offset == hardware_offset && expected_errno == hardware_errno) {
      continue;
    }

    LogError("Size:", master_length, "Spec off:", spec.offset, "Spec whence:", spec.whence, "Start off:", previous_offset, "Expected off:", expected_offset,
             "DUT off:", hardware_offset, "Expected errno:", expected_errno, "DUT errno:", hardware_errno);
  }

  sceKernelClose(fd);
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
  Log("Note:", files_cloned_at_destination, "files are supposed to be compared");

  char master_read[65536] {DEFAULT_CHR};
  char master_getdirentries[65536] {DEFAULT_CHR};

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
    entry_counter++;
  }

  Log("Compared", entry_counter, "entries");
  return offset;
}

TEST(DirentTests, PFS_Consistency) {
  LogTest("<<<< PFS read and getdirentries consistency >>>>");
  Log("Note:", files_cloned_at_destination, "files are supposed to be compared");
  LogWarning("Note: This function does not translate d_type between PFS and user filesystems");

  char master_read[65536] {DEFAULT_CHR};
  char master_getdirentries[65536] {DEFAULT_CHR};

  auto master_read_size          = undump_file(output_pfs_read, master_read, 65536);
  auto master_getdirentries_size = undump_file(output_pfs_getdirentries, master_getdirentries, 65536);

  if (auto res = PFSComparator(master_read, master_getdirentries, 65536); master_getdirentries_size != res) {
    LogError("PFS read and getdirentries have a different dirent at", res, ":");
    LogError("PFS Read:\t   ", to_hex_string(master_read + res, 24));
    LogError("PFS Getdirentries: ", to_hex_string(master_read + res, 24));
    FAIL("PFS read and getdirentries returned different amount of data");
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

TEST(DirentTests, DumpEverythingRaw) {
  LogTest("<<<< Dump everything >>>>");

  int  fd_read {};
  int  fd_dump {};
  char buffer[65536];
  s64  tbr {};

  sceKernelMkdir(output_dir, 0777);

  //

  fd_read = sceKernelOpen(input_pfs, O_DIRECTORY | O_RDONLY, 0777);
  CHECK_COMPARE_TEXT(fd_read, >, 0, "Can't open input dir");
  add_fd(fd_read);

  //

  fd_dump = sceKernelOpen(output_pfs_read, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  do {
    memset(buffer, DEFAULT_8, 65536);
    tbr = sceKernelRead(fd_read, buffer, 65536);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "PFS read failed");
    LogTest("PFS read got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
    if (pfs_read_file_size_target != tbr) LogError(pfs_read_file_size_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  fd_dump = sceKernelOpen(output_pfs_getdirentries, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  CHECK_EQUAL_ZERO_TEXT(sceKernelLseek(fd_read, 0, 0), "Can't rewind directory");
  do {
    memset(buffer, DEFAULT_8, 65536);
    tbr = sceKernelGetdirentries(fd_read, buffer, 65536, nullptr);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "PFS sceKernelGetdirentries failed");
    LogTest("PFS sceKernelGetdirentries got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
    if (pfs_getdirentries_target != tbr) LogError(pfs_getdirentries_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
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
    memset(buffer, DEFAULT_8, 65536);
    tbr = sceKernelRead(fd_read, buffer, 65536);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "Normal read failed");
    LogTest("Normal read got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
    if (normal_read_target != tbr) LogError(normal_read_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //
  fd_dump = sceKernelOpen(output_normal_getdirentries, O_CREAT | O_TRUNC | O_WRONLY, 0777);
  add_fd(fd_dump);
  CHECK_COMPARE_TEXT(fd_dump, >, 0, "Can't open output dir");
  CHECK_EQUAL_ZERO_TEXT(sceKernelLseek(fd_read, 0, 0), "Can't rewind directory");
  do {
    memset(buffer, DEFAULT_8, 65536);
    tbr = sceKernelGetdirentries(fd_read, buffer, 65536, nullptr);
    CHECK_COMPARE_TEXT(tbr, >=, 0, "Normal sceKernelGetdirentries failed");
    LogTest("Normal sceKernelGetdirentries got", tbr, "bytes");
    if (tbr == 0) break;
    CHECK_EQUAL(tbr, sceKernelWrite(fd_dump, buffer, tbr));
    if (normal_getdirentries_target != tbr) LogError(normal_getdirentries_target, tbr, "Incorrect read size"); // not ready for multiples of buffer size
  } while (tbr > 0);
  CHECK_EQUAL_ZERO(sceKernelClose(fd_dump));

  //

  CHECK_EQUAL_ZERO(sceKernelClose(fd_read));
}
