#include "fs_test.h"
#include "log.h"

#include <CppUTest/CommandLineTestRunner.h>
#include <orbis/SystemService.h>

IMPORT_TEST_GROUP(DirentTests);

int main(int ac, char** av) {
  // No buffering
  setvbuf(stdout, NULL, _IONBF, 0);

  // VSCode extension can open this immediately, text file w/ colors
  sceKernelMkdir("/data/ender_conf", 0666);
  LogInit("/data/ender_conf/log.ansi");

  // Log tests start
  Log();
  Log("<<<< TESTS START >>>>");
  Log();

  Log("<<<< Available config options >>>>");
  Log("<<<< nofuzz            skip fuzzing tests");
  Log("<<<< noread            skip read");
  Log("<<<< nogetdirentries   skip getdirentries");
  Log("<<<< nonormal          skip normal FS");
  Log("<<<< nopfs             skip PFS");
  Log("<<<< nolseek           skip lseek");
  Log("Create an entry in /data/ender_conf to activate");
  Log();

  // Run file system tests
  int result = RUN_ALL_TESTS(ac, av);

  // Log tests end
  Log();
  Log("<<<< TESTS END >>>>");
  Log();

  LogEnd();

  sceSystemServiceLoadExec("EXIT", nullptr);
  return 0;
  // return result;
}
