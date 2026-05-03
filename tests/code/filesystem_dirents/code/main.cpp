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

  Log("<<<< Available config options >>>>");
  Log("<<<< nofuzz - skip fuzzing tests");
  Log("Create an entry in /data/ender_conf to activate");
  Log();

  // Run file system tests
  int result = RUN_ALL_TESTS(ac, av);

  // Log tests end
  Log();
  Log("<<<< TESTS END >>>>");
  Log();

  sceSystemServiceLoadExec("EXIT", nullptr);
  return 0;
  // return result;
}
