/* Support added to the original Apache-2.0 Root My Galaxy helper.
 * The transport and credential handling in su_daemon.c are preserved.
 */
#include <sys/utsname.h>

extern const unsigned char rmg331_ksud_start[], rmg331_ksud_end[];
static char rmg331_loader_path[PATH_MAX];

static int rmg331_verify_defex(void) {
  const char *path = "/sys/module/kernelsu/parameters/samsung_defex_broad_root";
  char value[8] = {0};
  int fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
  if (fd < 0) {
    dprintf(STDERR_FILENO, "[rmg331] DEFEX verification failed: parameter unavailable errno=%d\n", errno);
    return 42;
  }
  ssize_t n = read(fd, value, sizeof(value)-1);
  close(fd);
  if (n < 1 || (value[0] != 'Y' && value[0] != '1')) {
    fd = open(path, O_WRONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) return 42;
    n = write(fd, "1\n", 2);
    close(fd);
    if (n != 2) return 42;
    fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) return 42;
    n = read(fd, value, sizeof(value)-1);
    close(fd);
    if (n < 1 || (value[0] != 'Y' && value[0] != '1')) return 42;
  }
  dprintf(STDOUT_FILENO,
          "[rmg331] DEFEX broad-root bypass ENABLED (samsung_defex_broad_root=Y)\n"
          "[rmg331] includes uid-0 tasks outside the KernelSU domain, as in HMA issue #137\n"
          "[rmg331] system_server/HMA functionality still needs a device test after userspace restart\n");
  return 0;
}

static int rmg331_write_all(int fd, const void *data, size_t length) {
  const unsigned char *p = data;
  while (length) {
    ssize_t n = write(fd, p, length);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0) return 0;
    p += n;
    length -= (size_t)n;
  }
  return 1;
}

static int rmg331_write_blob(const char *path) {
  int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0700);
  if (fd < 0) return 0;
  size_t length = (size_t)(rmg331_ksud_end-rmg331_ksud_start);
  int ok = rmg331_write_all(fd, rmg331_ksud_start, length);
  if (ok) ok = fchmod(fd, 0755) == 0 && fsync(fd) == 0;
  if (close(fd) != 0) ok = 0;
  if (!ok) return 0;
  fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
  if (fd < 0) return 0;
  unsigned char buffer[16384];
  size_t pos = 0;
  while (pos < length) {
    size_t want = length-pos;
    if (want > sizeof(buffer)) want = sizeof(buffer);
    ssize_t n = read(fd, buffer, want);
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0 || memcmp(buffer, rmg331_ksud_start+pos, (size_t)n) != 0) {
      close(fd);
      return 0;
    }
    pos += (size_t)n;
  }
  close(fd);
  return 1;
}

static int rmg331_backup_daemon(const char *destination) {
  int source = open("/data/adb/ksud", O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
  if (source < 0) return errno == ENOENT;
  struct stat st;
  if (fstat(source, &st) != 0 || !S_ISREG(st.st_mode) ||
      st.st_size < 0 || st.st_size > 64*1024*1024) {
    close(source);
    return 0;
  }
  int backup = open(destination, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0700);
  if (backup < 0) { close(source); return 0; }
  unsigned char buffer[16384];
  off_t copied = 0;
  int ok = 1;
  for (;;) {
    ssize_t n = read(source, buffer, sizeof(buffer));
    if (n < 0 && errno == EINTR) continue;
    if (n < 0) { ok = 0; break; }
    if (n == 0) break;
    if (!rmg331_write_all(backup, buffer, (size_t)n)) { ok = 0; break; }
    copied += n;
  }
  if (copied != st.st_size || fsync(backup) != 0) ok = 0;
  close(source);
  if (close(backup) != 0) ok = 0;
  if (ok) dprintf(STDOUT_FILENO, "[rmg331] previous /data/adb/ksud saved to %s\n", destination);
  return ok;
}

static int rmg331_prepare_loader(void) {
  struct utsname kernel;
  if (geteuid() != 0 || uname(&kernel) != 0 ||
      strncmp(kernel.release, "6.1.162-", 8) != 0 ||
      (!strstr(kernel.release, "abS928BXXU6EZIF") &&
       !strstr(kernel.release, "abS928BXXU6ZZI4"))) {
    dprintf(STDERR_FILENO, "[rmg331] STOP: root/release precondition failed\n");
    return 0;
  }
  char directory[] = "/data/local/tmp/rmg331-load-XXXXXX";
  if (!mkdtemp(directory)) return 0;
  char backup[PATH_MAX];
  if (snprintf(backup, sizeof(backup), "%s/ksud-before", directory) >= (int)sizeof(backup) ||
      snprintf(rmg331_loader_path, sizeof(rmg331_loader_path), "%s/ksud-3.3.1", directory) >=
          (int)sizeof(rmg331_loader_path)) return 0;
  /* Preserve an installed daemon before the upstream late-load moves its replacement. */
  if (!rmg331_backup_daemon(backup)) {
    dprintf(STDERR_FILENO, "[rmg331] STOP: cannot preserve existing daemon errno=%d\n", errno);
    return 0;
  }
  if (!rmg331_write_blob(rmg331_loader_path)) return 0;
  char temporary[PATH_MAX];
  if (snprintf(temporary, sizeof(temporary), "%s/stage", directory) >= (int)sizeof(temporary)) return 0;
  if (!rmg331_write_blob(temporary) ||
      rename(temporary, "/data/local/tmp/.ksud-stage") != 0) return 0;
  /* Maintenance actions may consult Next's staged path after late-load. Keep it
   * consistent too, retaining shell ownership so a later ordinary run can replace it. */
  if (!rmg331_write_blob(temporary) || chown(temporary, 2000, 2000) != 0 ||
      rename(temporary, "/data/local/tmp/ksud-s25u-kdp") != 0) return 0;
  dprintf(STDOUT_FILENO,
          "[rmg331] selected embedded ksud 3.3.1 sha256=3487bde89b33043bd6aa09db68d21a6375aaa126e131fda53b6f6318327b0081\n"
          "[rmg331] stage and maintenance copy replaced AFTER Next's copy; loader=%s\n"
          "[rmg331] embedded 6.1 module declares 6.1.166-dirty; phone compatibility remains experimental\n",
          rmg331_loader_path);
  return 1;
}
