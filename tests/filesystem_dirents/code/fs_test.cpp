#include "fs_test.h"

#include "orbis/UserService.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace FS_Test {
namespace oi = OrbisInternals;

void Drop(char* buffer, size_t size) {
  std::stringstream out;
  for (int b = 1; b <= size; b++) {
    out << std::setw(2) << std::hex << (0xFF & static_cast<unsigned int>(buffer[b - 1])) << " ";
    if ((b % 64) == 0) {
      out.flush();
      Log(out.str());
      std::stringstream().swap(out);
    }
  }
  Log(out.str(), "\n");
}

void DumpDirents(int fd, int buffer_size, s64 offset) {
  int   status {};
  s64   idx {};
  char* buffer = new char[buffer_size] {0};

  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp < 0) LogError("Lseek failed:", _tmp);
  idx = 0;
  do {
    memset(buffer, 0xAA, buffer_size);
    status = sceKernelRead(fd, buffer, buffer_size);
    Log("Read got", status, "bytes, idx=", idx, ",ptr=", sceKernelLseek(fd, 0, 1));
    if (status <= 0) break;
    Drop(buffer, buffer_size);
  } while (status > 0);

  if (int _tmp = sceKernelLseek(fd, offset, 0); _tmp < 0) LogError("Lseek failed:", _tmp);
  idx = 0;
  do {
    memset(buffer, 0xAA, buffer_size);
    status = sceKernelGetdirentries(fd, buffer, buffer_size, &idx);
    Log("GetDirEntries got", status, "bytes, idx=", idx, ",ptr=", sceKernelLseek(fd, 0, 1));
    if (status <= 0) break;
    Drop(buffer, buffer_size);
  } while (status > 0);
}

void RunTests() {
  RegenerateDir("/data/enderman");
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname01", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname02", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname03", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname04", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname05", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname06", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname07", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname08", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname09", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname10", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname11", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname12", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname13", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname14", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname15", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname16", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname17", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname18", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname19", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname10", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname21", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname22", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname23", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname24", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname25", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname26", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname27", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname28", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname29", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname30", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname31", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname32", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname33", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname34", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname35", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname36", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname37", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname38", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname39", O_CREAT | O_WRONLY | O_TRUNC, 0777));
  sceKernelClose(sceKernelOpen("/data/enderman/filewithaverylongname40", O_CREAT | O_WRONLY | O_TRUNC, 0777));

#define FUCKING_LSEEK_OFFSET 1024

  Log("---------------------");
  Log("Dump normal directory");
  Log("---------------------");

  int fd = sceKernelOpen("/data/enderman", O_DIRECTORY | O_RDONLY, 0777);
  Log("Directory opened with fd=", fd);

  Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
  Log("LSeek END+123456=", sceKernelLseek(fd, 123456, 2));
  Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
  Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

  Log("TEST: /data/enderman, 512 buffer, 0 offset");
  DumpDirents(fd, 512, 0);
  Log("TEST: /data/enderman, 512 buffer, 5 offset");
  DumpDirents(fd, 512, 5);
  Log("TEST: /data/enderman, 512 buffer, 30 offset");
  DumpDirents(fd, 512, 30);
  Log("TEST: /data/enderman, 768 buffer, 0 offset");
  DumpDirents(fd, 768, 0);
  Log("TEST: /data/enderman, 768 buffer, 5 offset");
  DumpDirents(fd, 768, 5);
  Log("TEST: /data/enderman, 768 buffer, 30 offset");
  DumpDirents(fd, 768, 30);
  Log("TEST: /data/enderman, 1024 buffer, 0 offset");
  DumpDirents(fd, 1024, 0);
  Log("TEST: /data/enderman, 1024 buffer, 5 offset");
  DumpDirents(fd, 1024, 5);
  Log("TEST: /data/enderman, 1024 buffer, 30 offset");
  DumpDirents(fd, 1024, 30);

  sceKernelClose(fd);

  Log("------------------");
  Log("Dump PFS directory");
  Log("------------------");
  fd = sceKernelOpen("/app0/assets/misc", O_DIRECTORY | O_RDONLY, 0777);

  Log("Directory opened with fd=", fd);

  Log("LSeek END+0=", sceKernelLseek(fd, 0, 2));
  Log("LSeek END+123456=", sceKernelLseek(fd, 123456, 2));
  Log("LSeek END+100=", sceKernelLseek(fd, 100, 2));
  Log("LSeek END-100000=", sceKernelLseek(fd, -100000, 2));

  // These aren't really needed, it's assumed PFS is only read in 65536 chunks
  //   Log("TEST: /app0/assets/misc, 512 buffer, 0 offset");
  //   DumpDirents(fd, 512, 0);
  //   Log("TEST: /app0/assets/misc, 512 buffer, 5 offset");
  //   DumpDirents(fd, 512, 5);
  //   Log("TEST: /app0/assets/misc, 550 buffer, 0 offset");
  //   DumpDirents(fd, 550, 0);
  //   Log("TEST: /app0/assets/misc, 550 buffer, 5 offset");
  //   DumpDirents(fd, 550, 5);
  //   Log("TEST: /app0/assets/misc, 768 buffer, 0 offset");
  //   DumpDirents(fd, 768, 0);
  //   Log("TEST: /app0/assets/misc, 768 buffer, 5 offset");
  //   DumpDirents(fd, 768, 5);

  Log("TEST: /app0/assets/misc, 65536 buffer, 0 offset");
  DumpDirents(fd, 65536, 0);
  Log("TEST: /app0/assets/misc, 65536 buffer, 5 offset");
  DumpDirents(fd, 65536, 5);
  Log("TEST: /app0/assets/misc, 65536 buffer, 30 offset");
  DumpDirents(fd, 65536, 30);

  sceKernelClose(fd);
}

bool RegenerateDir(const char* path) {
  Obliterate(path);
  sceKernelMkdir(path, 0777);
  return true;
}

} // namespace FS_Test