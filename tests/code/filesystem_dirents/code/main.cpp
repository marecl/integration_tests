#include "fs_test.h"
#include "log.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <orbis/SystemService.h>

IMPORT_TEST_GROUP(DirentTests);

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  // Log tests start
  Log();
  Log("<<<< TESTS START >>>>");
  Log();

  // prepare files
  RegenerateDir("/data/enderman");
  sceKernelMkdir("/data/enderman/files", 0777);
  sceKernelMkdir("/data/enderman/dump_pfs_read", 0777);
  sceKernelMkdir("/data/enderman/dump_pfs_getdents", 0777);
  sceKernelMkdir("/data/enderman/dump_data_read", 0777);
  sceKernelMkdir("/data/enderman/dump_data_getdents", 0777);

  // Run file system tests
  // int result = RUN_ALL_TESTS(ac, av);
  RunTests();

  // Log tests end
  Log();
  Log("<<<< TESTS END >>>>");
  Log();

  sceSystemServiceLoadExec("EXIT", nullptr);
  return 0;
  // return result;
}
