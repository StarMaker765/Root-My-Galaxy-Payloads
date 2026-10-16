# KernelSU 3.3.1 for the S24 Ultra payload

`ksud-s24-ultra` is the unchanged Samsung-patched KernelSU 3.3.1 daemon used
by the EZIF payload. SHA-256:

```text
3487bde89b33043bd6aa09db68d21a6375aaa126e131fda53b6f6318327b0081
```

It embeds four KMI modules. The 6.1 component is included separately as
`android14-6.1_kernelsu-s24-ultra.ko` for auditing, not as a second installation
step. It is 442264 bytes, with SHA-256:

```text
568eb54d4631a6b8aa60a69bd5ea65135376dee04da56215d9ca5dc257bce2f6
```

Its vermagic starts with `6.1.166-dirty`. The upstream manual loader handles
vermagic differences, which explains how it loaded on the owner's 6.1.162
kernel. Editing that string alone would not establish binary compatibility.
No dedicated 6.1.162 module or new daemon was compiled for this correction.

Static checks against the supplied original EZIF boot image found all 224
undefined module symbols, matched the seven examined Samsung KCFI signatures
and the examined credential offsets. These checks cover selected interfaces;
they do not establish complete ABI compatibility or runtime stability.
See [validation](../docs/VALIDATION.md).

The [Samsung patch](patches/KernelSU-v3.3.1-samsung-kdp-rkp-defex.patch) is retained
with its upstream source reference in [third-party notices](../THIRD_PARTY_NOTICES.md).
The tools directory retains the module/target audit scripts. The wrapper build
uses the pinned daemon binary and does not rebuild this patch or module.
