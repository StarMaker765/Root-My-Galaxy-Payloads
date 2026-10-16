#ifndef RMG333_KSU_STATE_H
#define RMG333_KSU_STATE_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* KSU release builds remove their sysfs kobject after init. /proc/modules
 * remains visible on the supported EZIF firmware, including from Shizuku.
 * Return 1 for present, 0 for absent, and -1 if the state cannot be read.
 */
static int rmg333_module_state_from_stream(FILE *stream) {
  char line[512];
  int line_start = 1;
  while (fgets(line, sizeof(line), stream)) {
    if (line_start && strncmp(line, "kernelsu", 8) == 0 &&
        (line[8] == ' ' || line[8] == '\t'))
      return 1;
    /* A dependency list may exceed the buffer. Its continuation is not the
     * start of a module record and must not be mistaken for a module name. */
    line_start = strchr(line, '\n') != NULL;
  }
  if (ferror(stream)) {
    if (!errno) errno = EIO;
    return -1;
  }
  return 0;
}

static int rmg333_module_state_from_path(const char *path) {
  FILE *stream = fopen(path, "r");
  if (!stream) return -1;
  errno = 0;
  int state = rmg333_module_state_from_stream(stream);
  int saved_errno = errno;
  if (fclose(stream) != 0 && state == 0) return -1;
  errno = saved_errno;
  return state;
}

static int rmg333_kernel_module_state(void) {
  return rmg333_module_state_from_path("/proc/modules");
}

#endif
