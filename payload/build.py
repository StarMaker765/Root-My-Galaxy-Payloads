#!/usr/bin/env python3
"""Rebuild only the r3 wrapper/helper, preserving the audited exploit and ksud."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

from elftools.elf.elffile import ELFFile

EXPECTED_EXPLOIT = '4f4255f51618e0a4044f383234f72822128e1efbca78e41149416afb52b73566'
EXPECTED_DAEMON = '3487bde89b33043bd6aa09db68d21a6375aaa126e131fda53b6f6318327b0081'

def digest(data):
    return hashlib.sha256(data).hexdigest()

def extract_component(path, prefix):
    with path.open('rb') as stream:
        elf = ELFFile(stream)
        symbols = {}
        for section in elf.iter_sections():
            if section.name in ('.symtab', '.dynsym'):
                symbols.update({s.name:s for s in section.iter_symbols()})
        start, end = symbols[prefix+'_start'], symbols[prefix+'_end']
        if start['st_shndx'] != end['st_shndx']:
            raise ValueError('Component crosses ELF sections')
        section = elf.get_section(start['st_shndx'])
        first = start['st_value']-section['sh_addr']
        last = end['st_value']-section['sh_addr']
        if not (0 <= first < last <= section['sh_size']):
            raise ValueError('Invalid component bounds')
        return section.data()[first:last]

def assembly(path, components):
    lines = ['.section .rodata', '.balign 16']
    for prefix, binary in components:
        name = binary.resolve().as_posix()
        if '"' in name or '\n' in name:
            raise ValueError('Unsupported path')
        lines += [f'.global {prefix}_start', f'.hidden {prefix}_start',
                  f'.global {prefix}_end', f'.hidden {prefix}_end',
                  f'{prefix}_start:', f'.incbin "{name}"', f'{prefix}_end:', '.balign 16']
    path.write_text('\n'.join(lines)+'\n', encoding='utf-8', newline='\n')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ndk', type=Path, required=True)
    args = parser.parse_args()
    here = Path(__file__).resolve().parent
    repo = here.parents[0]
    sources = here/'sources'
    work = repo/'build/e3q-S928BXXU6EZIF-r3'
    work.mkdir(parents=True, exist_ok=True)
    original = (here/'original-exploit.so').read_bytes()
    if digest(original) != EXPECTED_EXPLOIT:
        raise ValueError('Original exploit digest mismatch')
    daemon = repo/'kernelsu/ksud-s24-ultra'
    if digest(daemon.read_bytes()) != EXPECTED_DAEMON:
        raise ValueError('Audited daemon digest mismatch')
    exploit = work/'original-exploit.so'
    exploit.write_bytes(original)
    host = 'windows-x86_64' if os.name == 'nt' else 'darwin-x86_64' if sys.platform == 'darwin' else 'linux-x86_64'
    toolchain = args.ndk.resolve()/f'toolchains/llvm/prebuilt/{host}'
    compiler = toolchain/('bin/clang.exe' if os.name == 'nt' else 'bin/clang')
    compiler_version = subprocess.check_output([str(compiler),'--version'],text=True).splitlines()[0]
    common = [str(compiler),'--target=aarch64-linux-android35',
              '--sysroot='+str(toolchain/'sysroot'), '-O2','-g0','-Wall','-Wextra','-Werror',
              '-Wno-unused-parameter','-Wno-sign-compare','-fstack-protector-strong',
              '-ffunction-sections','-fdata-sections','-Wl,--gc-sections',
              '-Wl,-z,max-page-size=16384','-Wl,-z,relro','-Wl,-z,now',
              '-Wl,-z,noexecstack','-Wl,--build-id=sha1','-I'+str(sources)]
    daemon_asm = work/'daemon.S'
    assembly(daemon_asm,[('rmg331_ksud',daemon)])
    helper = work/'root-helper-r3'
    subprocess.run(common+['-fPIE','-pie',str(sources/'su_daemon-rmg332.c'),
                          str(daemon_asm),'-ldl','-o',str(helper)],check=True)
    payload_asm = work/'components.S'
    assembly(payload_asm,[('rmg331_exploit',exploit),('rmg331_helper',helper)])
    output = here/'payload.so'
    subprocess.run(common+['-fPIC','-shared',str(sources/'payload.c'),
                          str(payload_asm),'-ldl','-o',str(output)],check=True)
    if extract_component(output,'rmg331_exploit') != original:
        raise ValueError('Output changed the original exploit')
    if extract_component(helper,'rmg331_ksud') != daemon.read_bytes():
        raise ValueError('Output changed the audited daemon')
    with output.open('rb') as stream:
        elf = ELFFile(stream)
        if elf['e_machine'] != 'EM_AARCH64': raise ValueError('Wrong output architecture')
        if any(s['p_align'] < 16384 for s in elf.iter_segments() if s['p_type']=='PT_LOAD'):
            raise ValueError('Output alignment is below 16 KiB')
    record = {'revision':'r3 checks fix', 'artifact':output.name,
              'bytes':output.stat().st_size,'sha256':digest(output.read_bytes()),
              'target_model':'SM-S928B','audited_kernel_release':'6.1.162-android14-11-34343818-abS928BXXU6EZIF',
              'compiler':compiler_version,'compiler_target':'aarch64-linux-android35',
              'helper_sha256':digest(helper.read_bytes()),
              'exploit_sha256':EXPECTED_EXPLOIT,'daemon_sha256':EXPECTED_DAEMON,
              'daemon_changed':False,'embedded_module_recompiled':False,
              'embedded_module_vermagic':'6.1.166-dirty',
              'preserves_r2_private_module_isolation':True,
              'device_test_of_r3_performed':False}
    (here/'provenance.json').write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps(record,indent=2))

if __name__ == '__main__':
    main()
