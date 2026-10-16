#include "rmg333_ksu_state.h"

#ifdef _WIN32
__declspec(dllexport)
#endif
int rmg333_test_module_state(const char *path) {
  return rmg333_module_state_from_path(path);
}
