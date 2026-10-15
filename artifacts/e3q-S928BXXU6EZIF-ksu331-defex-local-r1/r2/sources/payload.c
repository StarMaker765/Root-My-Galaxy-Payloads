#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <sys/utsname.h>
#include <unistd.h>

extern const unsigned char rmg331_exploit_start[], rmg331_exploit_end[];
extern const unsigned char rmg331_helper_start[], rmg331_helper_end[];

static void fail(const char *message) {
  dprintf(STDERR_FILENO, "[rmg331] STOP: %s (errno=%d)\n", message, errno);
  _exit(41);
}

static void write_component(const char *path, const unsigned char *data,
                            size_t length, mode_t mode) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
  if (fd < 0) fail("cannot create private component");
  /* Mark the new file read-only before writing through the already open FD. */
  if (fchmod(fd, mode) != 0) fail("cannot set component permissions");
  size_t pos = 0;
  while (pos < length) {
    ssize_t n = write(fd, data + pos, length - pos);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0) fail("cannot write component");
    pos += (size_t)n;
  }
  if (fsync(fd) != 0 || close(fd) != 0) fail("cannot flush component");
  fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
  if (fd < 0) fail("cannot verify component");
  unsigned char buffer[16384];
  pos = 0;
  while (pos < length) {
    size_t want = length - pos;
    if (want > sizeof(buffer)) want = sizeof(buffer);
    ssize_t n = read(fd, buffer, want);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0 || memcmp(buffer, data + pos, (size_t)n) != 0)
      fail("component read-back mismatch");
    pos += (size_t)n;
  }
  close(fd);
}

__attribute__((constructor)) static void rmg331_start(void) {
  static int started;
  if (started) return;
  started = 1;
  unsetenv("LD_PRELOAD");
  dprintf(STDERR_FILENO,
          "[rmg331] DIAGNOSTIC r2: original ZZI4 exploit + same KSU 3.3.1; loader without modules\n");
  if (getuid() != 2000 || geteuid() != 2000)
    fail("enable Shizuku/shell mode in Root My Galaxy Next for this test");
  char model[PROP_VALUE_MAX] = {0};
  __system_property_get("ro.product.model", model);
  if (strcmp(model, "SM-S928B") != 0)
    fail("this build is restricted to SM-S928B");
  struct utsname kernel;
  if (uname(&kernel) != 0 || strcmp(kernel.machine, "aarch64") != 0 ||
      strncmp(kernel.release, "6.1.162-", 8) != 0 ||
      (!strstr(kernel.release, "abS928BXXU6EZIF") &&
       !strstr(kernel.release, "abS928BXXU6ZZI4")))
    fail("kernel release differs from the EZIF/ZZI4 6.1.162 test target");
  struct stat loaded;
  if (stat("/sys/module/kernelsu", &loaded) == 0)
    fail("KernelSU is already loaded; reboot without automatic root first");
  const char *original_helper = getenv("CVE43499_ROOT_HELPER");
  if (!original_helper || original_helper[0] != '/')
    fail("run this file through Root My Galaxy Next, not as an ordinary APK library");

  char directory[] = "/data/local/tmp/rmg331-test-XXXXXX";
  if (!mkdtemp(directory)) fail("cannot create private test directory");
  char exploit[PATH_MAX], helper[PATH_MAX];
  if (snprintf(exploit, sizeof(exploit), "%s/zzi4-original.so", directory) >= (int)sizeof(exploit) ||
      snprintf(helper, sizeof(helper), "%s/root-helper", directory) >= (int)sizeof(helper))
    fail("private component path too long");
  write_component(exploit, rmg331_exploit_start,
                  (size_t)(rmg331_exploit_end-rmg331_exploit_start), 0444);
  write_component(helper, rmg331_helper_start,
                  (size_t)(rmg331_helper_end-rmg331_helper_start), 0555);
  if (setenv("CVE43499_ROOT_HELPER", helper, 1) != 0)
    fail("cannot select the 3.3.1 test helper");
  dprintf(STDERR_FILENO, "[rmg331] kernel=%s\n[rmg331] work=%s\n",
          kernel.release, directory);
  dprintf(STDERR_FILENO,
          "[rmg331] device-module compatibility is UNCONFIRMED; no boot partition is written\n");
  /* The known working exploit is kept byte-for-byte; its constructor owns the run. */
  if (!dlopen(exploit, RTLD_NOW | RTLD_LOCAL)) {
    dprintf(STDERR_FILENO, "[rmg331] dlopen: %s\n", dlerror());
    fail("cannot load the original ZZI4 exploit");
  }
  dprintf(STDERR_FILENO, "[rmg331] original exploit constructor returned\n");
}
