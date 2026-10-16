# Online payload source

Source: `StarMaker765/Root-My-Galaxy-Payloads`, branch `main`.
Feed: [targets-v3.json](targets-v3.json).

The v3 catalog advertises only **S24 Ultra | KernelSU 3.3.1**, for SM-S928B
firmware S928BXXU6EZIF. Both artifacts are hosted in this repository and have
explicit byte sizes and SHA-256 digests. Keep application integrity checks on.

The historical profile ID `e3q-S928BXXU6EZIF-ksu331-defex-local-r1` remains
stable. It is an internal identifier, not the visible revision or a local-only
installation requirement. Remove an old Local payload override and refresh
the source before testing the current Online files. A stale offline copy is
not evidence that the new revision was downloaded.

The wrapper enforces the full kernel release even though the application may
also match the short `6.1.162` kernel string. No other device/firmware profiles
are advertised. Next 0.13 uses this v3 feed; the inherited v2 catalog is removed.
