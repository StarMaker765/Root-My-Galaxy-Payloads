/* Diagnostic-only module isolation in the loader's private mount namespace.
 * No original module directories or disable/remove flags are changed.
 */
#include <dirent.h>
#include <stdarg.h>
#include <time.h>

static int rmg332_log_fd = -1;
static char rmg332_diag_dir[PATH_MAX];
static const char *const rmg332_hidden_dirs[] = {
    "/data/adb/modules", "/data/adb/modules_update", "/data/adb/modules_rmg_backup",
    "/data/adb/late-load.d", "/data/adb/post-mount.d", "/data/adb/service.d",
    "/data/adb/boot-completed.d", "/data/adb/post-fs-data.d",
};

static int rmg332_phase(const char *format, ...) {
  if (rmg332_log_fd < 0) return 0;
  struct timespec now;
  if (clock_gettime(CLOCK_BOOTTIME, &now) != 0) return 0;
  if (dprintf(rmg332_log_fd, "[%lld.%03ld] pid=%d ",
              (long long)now.tv_sec, now.tv_nsec / 1000000L, getpid()) < 0) return 0;
  va_list args;
  va_start(args, format);
  int result = vdprintf(rmg332_log_fd, format, args);
  va_end(args);
  if (result < 0 || dprintf(rmg332_log_fd, "\n") < 0) return 0;
  return fsync(rmg332_log_fd) == 0;
}

static int rmg332_begin(void) {
  char directory[] = "/data/local/tmp/rmg332-diag-XXXXXX";
  if (!mkdtemp(directory)) return 0;
  if (chmod(directory, 0755) != 0) return 0;
  if (snprintf(rmg332_diag_dir, sizeof(rmg332_diag_dir), "%s", directory) >=
      (int)sizeof(rmg332_diag_dir)) return 0;
  char log_path[PATH_MAX];
  if (snprintf(log_path, sizeof(log_path), "%s/phase.log", directory) >= (int)sizeof(log_path)) return 0;
  rmg332_log_fd = open(log_path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC | O_APPEND, 0600);
  if (rmg332_log_fd < 0 || fchmod(rmg332_log_fd, 0644) != 0) return 0;
  dprintf(STDOUT_FILENO, "[rmg332] phase log: %s\n", log_path);
  return rmg332_phase("ROOT_READY diagnostic_r2 modules_isolated_for_loader_only");
}

static int rmg332_directory_empty(const char *path) {
  DIR *directory = opendir(path);
  if (!directory) return 0;
  int empty = 1;
  for (;;) {
    errno = 0;
    struct dirent *entry = readdir(directory);
    if (!entry) {
      if (errno) empty = 0;
      break;
    }
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      empty = 0;
      break;
    }
  }
  closedir(directory);
  return empty;
}

static int rmg332_validate_metamodule(void) {
  struct stat st;
  if (lstat("/data/adb/metamodule", &st) != 0) return errno == ENOENT;
  /* KernelSU v3.3.1 only accepts a symlink here; otherwise it searches modules. */
  if (!S_ISLNK(st.st_mode)) return 1;
  char resolved[PATH_MAX];
  if (!realpath("/data/adb/metamodule", resolved)) return errno == ENOENT;
  for (size_t i = 0; i < 3; ++i) {
    size_t length = strlen(rmg332_hidden_dirs[i]);
    if (strncmp(resolved, rmg332_hidden_dirs[i], length) == 0 && resolved[length] == '/') return 1;
  }
  /* Never print a private module name or operate on an unexpected target. */
  dprintf(STDERR_FILENO, "[rmg332] STOP: metamodule target is outside the module directories\n");
  return 0;
}

static int rmg332_isolate_modules(void) {
  if (!rmg332_validate_metamodule()) return 0;
  int masked = 0;
  for (size_t i = 0; i < sizeof(rmg332_hidden_dirs)/sizeof(rmg332_hidden_dirs[0]); ++i) {
    const char *target = rmg332_hidden_dirs[i];
    struct stat st;
    if (lstat(target, &st) != 0) {
      if (errno != ENOENT) return 0;
      continue;
    }
    if (!S_ISDIR(st.st_mode)) {
      dprintf(STDERR_FILENO, "[rmg332] STOP: expected a real directory at a standard module path\n");
      return 0;
    }
    char empty_path[PATH_MAX];
    if (snprintf(empty_path, sizeof(empty_path), "%s/empty-%zu", rmg332_diag_dir, i) >=
        (int)sizeof(empty_path)) return 0;
    if (mkdir(empty_path, 0700) != 0 || mount(empty_path, target, NULL, MS_BIND, NULL) != 0) return 0;
    if (!rmg332_directory_empty(target)) return 0;
    ++masked;
    if (!rmg332_phase("DIRECTORY_MASKED index=%zu original_not_renamed_or_deleted", i)) return 0;
  }
  /* A symlink to a hidden module must no longer resolve inside this namespace. */
  char resolved[PATH_MAX];
  struct stat meta;
  if (lstat("/data/adb/metamodule", &meta) == 0 && S_ISLNK(meta.st_mode) &&
      realpath("/data/adb/metamodule", resolved)) return 0;
  dprintf(STDOUT_FILENO,
          "[rmg332] DIAGNOSTIC: %d module/script directories hidden only from the loader\n"
          "[rmg332] original module directories, updates and backups were not moved or deleted\n", masked);
  return rmg332_phase("ISOLATION_READY masked=%d namespace_private=1", masked);
}
