# SM-S928B EZIF validation

Review date: 2026-10-15. Previous payload source:
`f0434022997843f18c9084188114b6002754b285`.

## Device observations

The supplied Next 0.13 installation log verified downloads, obtained root on
attempt 1/24, reported installation in 17 seconds and saved an offline copy.
Native KernelSU control and `su` worked despite the helper returning code 42.
The report showed `kernelsu` in `/proc/modules` with no `/sys/module/kernelsu`.

KernelSU v3.3.1 deliberately removes its sysfs kobject in non-debug LKM builds.
The old helper's mandatory sysfs DEFEX check therefore produced a false failure.
The same sysfs-based already-loaded guard was insufficient. The r3 sources fix
both checks while retaining the native version/flag verification.

The owner then uninstalled OverlayFS Enhanced 1.3.4 (`meta-overlayfsx`) and
reported a successful soft reboot with KernelSU still working on the previous
payload. This supports a failure involving that metamodule or its interaction
with other modules. It does not identify the precise panic cause. The supplied
collector report ran as shell UID 2000; dmesg and pstore were inaccessible,
so it contained no panic backtrace.

## Original firmware audit

| Input | Bytes | SHA-256 |
| --- | ---: | --- |
| Original EZIF boot.img | 100663296 | `b98326455c825062e5350ae88b75488f4de4c7b1e587a47105fffdc41be37500` |
| Extracted kernel | 38140416 | `a8c1638497ca1bff576c5a6d9426521ec545fd68da8ce5dfa47c40233391a179` |

The kernel banner matches `6.1.162-android14-11-34343818-abS928BXXU6EZIF`.
The submitted config.gz matches the compressed configuration in boot.img.
Recovered data included 138227 BTF types and 8346 exported symbol CRCs.
The original image, derived kernel and personal reports are not distributed here.

- All 224 undefined symbols of the embedded 6.1 module exist in the target;
  81 require kallsyms resolution. The empty `__versions` section follows the
  manual loader contract and is not a complete ABI comparison.
- The examined BTF signatures and KCFI IDs match `prepare_ro_creds`,
  `kdp_assign_pgd`, `kdp_usecount_dec_and_test`, `inc_rlimit_ucounts`,
  `dec_rlimit_ucounts`, `get_task_creds` and `set_task_creds`.
- Examined offsets match: task real_cred 0x830, task cred 0x838,
  cred user 0x80 and cred ucounts 0x90.
- `ksu_patch_text` returns -EOPNOTSUPP in the examined variant.
  CONFIG_OVERLAY_FS, CONFIG_EXT4_FS and CONFIG_BLK_DEV_LOOP are built in.

These findings address selected compatibility questions; they do not validate
every module structure, hook or reboot path.

## Local r3 checks

- ARM64 Android API 35 build, NDK r30, warnings treated as errors.
- ELF load segments aligned to 16 KiB; no executable stack.
- Thirteen compiled host regression cases for the shared module-state reader.
- Original exploit and daemon preserved byte for byte in the rebuilt artifact.
- Catalog sizes and SHA-256 digests checked against the files.

The r3 correction has not been installed or tested on the phone. Online
installation, offline caching and a soft reboot with the new artifact remain
device acceptance checks. The successful owner-reported reboot predates r3.

## Source references

- [KernelSU sysfs removal](https://github.com/tiann/KernelSU/blob/0c0fdb960178fa12d80d6cbcfdcb6ad8be6a865d/kernel/core/init.c)
- [KernelSU manual loader and vermagic](https://github.com/tiann/KernelSU/blob/0c0fdb960178fa12d80d6cbcfdcb6ad8be6a865d/userspace/ksuinit/src/lib.rs)
- [KernelSU soft reboot](https://github.com/tiann/KernelSU/blob/0c0fdb960178fa12d80d6cbcfdcb6ad8be6a865d/userspace/ksud/src/soft_reboot.rs)
- [KernelSU metamodule state](https://github.com/tiann/KernelSU/blob/0c0fdb960178fa12d80d6cbcfdcb6ad8be6a865d/userspace/ksud/src/metamodule.rs)
- [OverlayFS Enhanced 1.3.4](https://github.com/RipperHybrid/Meta-Overlayfsx/tree/fbd2ca8ec704ec15de7ae315cb15238ac9a1787b)
