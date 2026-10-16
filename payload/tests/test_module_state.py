#!/usr/bin/env python3
"""Host regression checks for the shared /proc/modules scanner (not Android runtime tests)."""
import argparse
import ctypes
import os
from pathlib import Path
import subprocess

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', required=True, type=Path)
    args = parser.parse_args()
    here = Path(__file__).resolve().parent
    repo = here.parents[1]
    work = repo/'build/e3q-S928BXXU6EZIF-r3/host-tests'
    work.mkdir(parents=True, exist_ok=True)
    library = work/('module-state.dll' if os.name == 'nt' else 'module-state.so')
    command = [str(args.cc.resolve()),'-shared','-I'+str(here.parent/'sources'),
               str(here/'module_state_host.c'),'-o',str(library)]
    if os.name != 'nt': command.insert(1,'-fPIC')
    subprocess.run(command,check=True)
    loaded = ctypes.CDLL(str(library))
    scanner = loaded.rmg333_test_module_state
    scanner.argtypes = [ctypes.c_char_p]
    scanner.restype = ctypes.c_int
    module = b'kernelsu 241664 1 - Live 0x0000000000000000 (O)\n'
    vendor = b'vendor_driver 4096 0 - Live 0x0000000000000000\n'
    prefix = b'vendor_driver 4096 1 '
    continuation = prefix + b'x'*(511-len(prefix)) + module
    cases = [
        ('empty',b'',0),
        ('stock-modules',vendor,0),
        ('loaded-despite-hidden-sysfs',vendor+module,1),
        ('first-record',module+vendor,1),
        ('last-record-without-newline',vendor+module.rstrip(b'\n'),1),
        ('tab-separated',b'kernelsu\t241664 1 - Live 0x0\n',1),
        ('different-module-prefix',b'kernelsu_next 241664 1 - Live 0x0\n',0),
        ('different-module-suffix',b'other_kernelsu 241664 1 - Live 0x0\n',0),
        ('dependency-only',b'vendor_driver 4096 1 kernelsu, Live 0x0\n',0),
        ('long-record-continuation',continuation+vendor,0),
        ('loaded-after-long-record',continuation+vendor+module,1),
        ('incomplete-name',b'kernelsu',0),
    ]
    for name,content,expected in cases:
        path = work/(name+'.txt')
        path.write_bytes(content)
        actual = scanner(os.fsencode(path))
        if actual != expected:
            raise AssertionError((name,actual,expected))
    absent = work/'missing-proc-modules'
    if absent.exists(): raise ValueError('Missing-file fixture must not exist')
    if scanner(os.fsencode(absent)) != -1:
        raise AssertionError('Unreadable state must not be treated as absent')
    print(f'PASS: {len(cases)+1} module-state regression cases; Android runtime validation remains pending')

if __name__ == '__main__':
    main()
