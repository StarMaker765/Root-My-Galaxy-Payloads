# SM-S928B / S928BXXU6EZIF payload

Published name: **S24 Ultra | KernelSU 3.3.1**. Internal correction revision: r3.

## Build

Install Python, the packages in the root `requirements.txt`, and Android NDK r30.
From this directory:

```sh
python build.py --ndk /path/to/android-ndk-r30
```

The builder compiles the wrapper and root helper for ARM64 Android API 35.
It pins and embeds `original-exploit.so` and `../../kernelsu/ksud-s24-ultra`,
checks their digests before building, then checks the embedded bytes again.
It does not compile the original exploit, daemon or kernel module from source.
The upstream source revisions are listed in the root third-party notices.

The published payload was built with NDK r30 (30.0.16248370), Clang 21.0.0.
`provenance.json` records the compiler, component digests and output digest.
`validation.json` records the local checks separately from device validation.

## Host regression checks

```sh
python tests/test_module_state.py --cc /path/to/host-c-compiler
```

These 13 checks compile the shared `/proc/modules` reader for the host and test
exact module names, dependencies, long lines and unreadable input. They are
not Android runtime or soft reboot tests.

## Behavior

- Both entry points check the first field of `/proc/modules` before loading.
  An unreadable file stops the attempt instead of assuming KernelSU is absent.
- Native control must report version 32680 and LKM/late-load flags.
- After native verification, an absent DEFEX sysfs parameter (ENOENT) is logged
  as unavailable and does not generate the former false error 42. It is not
  reported as a functional DEFEX test. Other read errors and an exposed disabled
  parameter remain failures. The helper no longer writes that parameter.
- Only the exact EZIF kernel release is accepted.
- Module/script isolation from r2 remains local to the loader mount namespace.
  Soft reboot still processes the real installed modules in the init namespace.
- The phase log is `/data/local/tmp/rmg333-diag-*/phase.log`. A successful native
  check is recorded as `CONTROL_VERIFIED`, followed by the DEFEX read result
  and `CONTROL_END code=0`.

## Diagnostics

Copy `collect-diagnostics.sh` to the phone and run it in an already available
shell. It collects accessible phase logs, kernel logs, module markers and
configuration into a new `RMG-S24-diagnostics-*` directory under Download
(or `/data/local/tmp`). It does not acquire root, execute the payload, change
modules or reboot. A shell without granted root may report permission failures.
Review collected reports before sharing them; raw reports are not in this repo.

See [validation](../../docs/VALIDATION.md) for the owner-reported soft reboot
result and the remaining device checks.
