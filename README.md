# Galaxy S24 Ultra Payload

**S24 Ultra | KernelSU 3.3.1** is a device-specific payload for
[Root My Galaxy Next](https://github.com/rushiranpise/Root-My-Galaxy-Next).
This repository publishes only the SM-S928B EZIF profile and its supporting files.

| Item | Supported target |
| --- | --- |
| Device | Samsung Galaxy S24 Ultra (SM-S928B) |
| Firmware | S928BXXU6EZIF |
| Kernel | `6.1.162-android14-11-34343818-abS928BXXU6EZIF` |
| KernelSU | 3.3.1, Samsung KDP/RKP/DEFEX variant |
| Application used in the installation report | Root My Galaxy Next 0.13+ci.53.1cc7bf0 |

## Online installation

1. In Root My Galaxy Next, set the GitHub payload source to
   `StarMaker765/Root-My-Galaxy-Payloads`, branch `main`, and refresh the source.
2. Select **S24 Ultra | KernelSU 3.3.1**. Remove any previous Local payload override
   so Online mode downloads the published files.
3. Use Shizuku and Install KernelSU. For the first run of this revision, keep
   automatic soft restart off and save the installation log.

If KernelSU is already loaded, perform a full device restart before attempting
a new installation. A soft reboot keeps the kernel and KernelSU loaded; the
payload deliberately refuses to load a second copy.

The [v3 feed](support/targets-v3.json) contains one profile and the actual file
sizes and SHA-256 digests. The internal profile ID is preserved so existing
selections retain their identity. Online download and offline cache behavior
still require confirmation on the device with this revision.

## Current correction

The r3 loader fixes a false error 42 caused by checking a KernelSU sysfs entry
that release builds deliberately remove. It verifies the native driver version
and flags first, distinguishes an unavailable DEFEX parameter from a failed
read, and detects an already loaded module through `/proc/modules`.

The exploit and Samsung `ksud` 3.3.1 are unchanged. This release does not include
a newly compiled 6.1.162 kernel module. The embedded 6.1 module still reports
`6.1.166-dirty`; see [the daemon notes](kernelsu/README.md) for the compatibility
audit and its limits.

The owner reported a successful soft reboot with KernelSU still active after
uninstalling OverlayFS Enhanced 1.3.4 on the previous payload. That implicates
the metamodule or its interaction with other modules; no panic backtrace was
available to identify the failing instruction. This payload does not repair
or validate that metamodule.

## Validation and files

- [Validation and device evidence](docs/VALIDATION.md)
- [Payload sources, build instructions and diagnostics](payload/README.md)
- [KernelSU daemon and Samsung patch](kernelsu/README.md)
- [Online source details](support/README.md)
- [Third-party credits and licenses](THIRD_PARTY_NOTICES.md)

The ARM64 build and 13 host regression cases passed. The r3 correction has not
yet been installed or tested on the device. Support is restricted to the exact
kernel release listed above; other S24 variants and firmware are not covered.
