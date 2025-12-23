#include "fs_test.h"

#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <orbis/UserService.h>
#include <sstream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;

namespace FS_Test {
namespace oi = OrbisInternals;



void RunTests() {
  RegenerateDir("/data/amphitheathre");

  Log();
  Log("<<<< TEST SUITE STARTING >>>>");
  Log("\tSome test components may (and will) fail. This is expected.");
  Log("\tValidity of the test is determined by last message emitted by test case.");
  Log();

  //

  Log();
  Log("\t<<<< RELATIVE FILENO >>>>");
  Log();

  touch("/data/amphitheathre/");

  
}


} // namespace FS_Test