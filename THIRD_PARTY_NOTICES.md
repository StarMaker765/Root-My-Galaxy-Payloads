# Third-party notices

This repository is a focused distribution of the SM-S928B EZIF payload.
Removing other device profiles does not remove the authorship or license
obligations of the components retained here.

## Root My Galaxy payload and helper

The root [LICENSE](LICENSE) and
[original helper license](payload/LICENSE-original-helper.txt)
retain the inherited Apache License 2.0 terms. The helper derives from
[Root My Galaxy Payloads, revision 6e3223e689688540060ddd97a0927e927bfed207](https://github.com/BuSung-dev/Root-My-Galaxy-Payloads/tree/6e3223e689688540060ddd97a0927e927bfed207).
Local wrapper/helper changes for this target are visible in Git history and
described in the payload README. Existing source copyright notices remain.

The embedded original exploit is an unchanged, SHA-256-pinned ARM64 binary
from the owner's preceding EZIF payload. The wrapper build does not rebuild
its native exploit implementation. The inherited exploit lineage credits
[NebuSec / CyberMeowfia, IonStack CVE-2026-43499](https://github.com/NebuSec/CyberMeowfia/tree/main/IonStack/CVE-2026-43499/exploit).
The exact preceding binary and source context remain accessible in
[revision f043402](https://github.com/StarMaker765/Root-My-Galaxy-Payloads/tree/f0434022997843f18c9084188114b6002754b285/artifacts/e3q-S928BXXU6EZIF-ksu331-defex-local-r1).

## KernelSU

KernelSU is developed by [tiann and contributors](https://github.com/tiann/KernelSU).
The daemon/module variant used here derives from KernelSU v3.3.1,
[revision 0c0fdb960178fa12d80d6cbcfdcb6ad8be6a865d](https://github.com/tiann/KernelSU/tree/0c0fdb960178fa12d80d6cbcfdcb6ad8be6a865d),
with the Samsung KDP/RKP/DEFEX patch distributed by
[rushiranpise's payload repository, revision a2b22606e5e681f8649cc18010f06447787a79bf](https://github.com/rushiranpise/Root-My-Galaxy-Payloads/tree/a2b22606e5e681f8649cc18010f06447787a79bf/kernelsu).
That pinned upstream directory provides the variant's build/source notes;
the same Samsung patch is retained in this repository.

KernelSU's [GPL-3.0 license](LICENSES/KernelSU-GPL-3.0.txt) and the kernel
component's [GPL-2.0 license](LICENSES/KernelSU-kernel-GPL-2.0.txt) are retained
separately. The Apache license for the payload helper does not relicense the
KernelSU binaries. The daemon and extracted reference module are unchanged.
