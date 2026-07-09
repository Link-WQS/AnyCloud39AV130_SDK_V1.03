#!/usr/bin/env python
"""
mbed SDK
Copyright (c) 2017-2019 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

LIBRARIES BUILD
"""

from __future__ import print_function
from os import path
import re
import bisect
from subprocess import check_output
import sys

#arm-none-eabi-nm -nl <elf file>
_NM_EXEC = "arm-none-eabi-nm"
_OPT = "-nlC"
_PTN = re.compile("([0-9a-f]*) ([Tt]) ([^\t\n]*)(?:\t(.*):([0-9]*))?")

class ElfHelper(object):
    def __init__(self, elf_file, map_file):

        op = check_output([_NM_EXEC, _OPT, elf_file.name]).decode('utf-8')
        self.maplines = map_file.readlines()
        self.matches = _PTN.findall(op)
        self.addrs = [int(x[0], 16) for x in self.matches]

    def function_addrs(self):
        return self.addrs

    def function_name_for_addr(self, addr):
        if addr >= self.addrs[0] and addr <= self.addrs[-1]:
            i = bisect.bisect_right(self.addrs, addr)
            funcname = self.matches[i-1][2]
            funcname = funcname.strip() \
            + u" (%08x + 0x%x/0x%x)" % (self.addrs[i-1], (addr - self.addrs[i-1]), ((self.addrs[i] if i < len(self.addrs) else self.addrs[-1]) - self.addrs[i-1]))
        else:
            funcname = u"<unknown-symbol>"
        return funcname

def print_HFSR_info(hfsr):
    if int(hfsr, 16) & 0x80000000:
        print("\t\tDebug Event Occurred")
    if int(hfsr, 16) & 0x40000000:
        print("\t\tForced exception, a fault with configurable priority has been escalated to HardFault")
    if int(hfsr, 16) & 0x2:
        print("\t\tVector table read fault has occurred")

def print_MMFSR_info(mmfsr, mmfar):
    if int(mmfsr, 16) & 0x20:
        print("\t\tA MemManage fault occurred during FP lazy state preservation")
    if int(mmfsr, 16) & 0x10:
        print("\t\tA derived MemManage fault occurred on exception entry")
    if int(mmfsr, 16) & 0x8:
        print("\t\tA derived MemManage fault occurred on exception return")
    if int(mmfsr, 16) & 0x2:
        if int(mmfsr, 16) & 0x80:
            print("\t\tData access violation. Faulting address: %s"%(str(mmfar)))
        else:
            print("\t\tData access violation. WARNING: Fault address in MMFAR is NOT valid")
    if int(mmfsr, 16) & 0x1:
        print("\t\tMPU or Execute Never (XN) default memory map access violation on an instruction fetch has occurred")

def print_BFSR_info(bfsr, bfar):
    if int(bfsr, 16) & 0x20:
        print("\t\tA bus fault occurred during FP lazy state preservation")
    if int(bfsr, 16) & 0x10:
        print("\t\tA derived bus fault has occurred on exception entry")
    if int(bfsr, 16) & 0x8:
        print("\t\tA derived bus fault has occurred on exception return")
    if int(bfsr, 16) & 0x4:
        print("\t\tImprecise data access error has occurred")
    if int(bfsr, 16) & 0x2:
        if int(bfsr,16) & 0x80:
            print("\t\tA precise data access error has occurred. Faulting address: %s"%(str(bfar)))
        else:
            print("\t\tA precise data access error has occurred. WARNING: Fault address in BFAR is NOT valid")
    if int(bfsr, 16) & 0x1:
        print("\t\tA bus fault on an instruction prefetch has occurred")

def print_UFSR_info(ufsr):
    if int(ufsr, 16) & 0x200:
        print("\t\tDivide by zero error has occurred")
    if int(ufsr, 16) & 0x100:
        print("\t\tUnaligned access error has occurred")
    if int(ufsr, 16) & 0x8:
        print("\t\tA coprocessor access error has occurred. This shows that the coprocessor is disabled or not present")
    if int(ufsr, 16) & 0x4:
        print("\t\tAn integrity check error has occurred on EXC_RETURN")
    if int(ufsr, 16) & 0x2:
        print("\t\tInstruction executed with invalid EPSR.T or EPSR.IT field( This may be caused by Thumb bit not being set in branching instruction )")
    if int(ufsr, 16) & 0x1:
        print("\t\tThe processor has attempted to execute an undefined instruction")

def print_CPUID_info(cpuid):
    if (int(cpuid, 16) & 0xF0000) == 0xC0000:
        print("\t\tProcessor Arch: ARM-V6M")
    else:
        print("\t\tProcessor Arch: ARM-V7M or above")

    print("\t\tProcessor Variant: %X" % ((int(cpuid,16) & 0xFFF0 ) >> 4))

def parse_line_for_register(line):
    _, register_val = line.split(":")
    return register_val.strip()

def parse_line_for_ram(line):
    _, register_val = line.split(":")
    return register_val.strip()

def print_stack_call_trace(adr_list, val_list, elfhelper):
    if elfhelper:
        for val in val_list:
            func_name = elfhelper.function_name_for_addr(int(val, 16))
            if func_name != u"<unknown-symbol>":
                print(" [%s] : %s" % (str(val), func_name.strip()))
    else:
        pass

def main(crash_log, elfhelper):
    mmfsr_val = 0
    mmfar_val = 0
    bfsr_val = 0
    bfar_val = 0
    ufsr_val = 0
    ram_str_list = []
    adr_str_list = []
    val_str_list = []
    lines = iter(crash_log.read().decode('utf-8').splitlines())

    for eachline in lines:
        if "++ CM4 Fault Handler ++" in eachline:
            break
    else:
        print("ERROR: Unable to find \"CM4 Fault Handler\" header")
        return

    for eachline in lines:
        if "-- CM4 Fault Handler --" in eachline:
            break

        elif eachline.startswith("PC"):
            pc_val = parse_line_for_register(eachline)
            if elfhelper:
                pc_name = elfhelper.function_name_for_addr(int(pc_val, 16))
            else:
                pc_name = "<unknown-symbol>"

        elif eachline.startswith("LR"):
            lr_val = parse_line_for_register(eachline)
            if elfhelper:
                lr_name = elfhelper.function_name_for_addr(int(lr_val, 16))
            else:
                lr_name = "<unknown-symbol>"

        elif eachline.startswith("SP"):
            sp_val = parse_line_for_register(eachline)
            sp_addr = int(sp_val, 16) & ~0xF
            sp_depthx4 = 16
            ram_addr_list = range(sp_addr, sp_addr + 16 * sp_depthx4, 16)
            ram_str_list = list(map(lambda x: "%08x" % x, ram_addr_list))

        elif eachline.startswith("HFSR"):
            hfsr_val = parse_line_for_register(eachline)

        elif eachline.startswith("MMFSR"):
            mmfsr_val = parse_line_for_register(eachline)

        elif eachline.startswith("BFSR"):
            bfsr_val = parse_line_for_register(eachline)

        elif eachline.startswith("UFSR"):
            ufsr_val = parse_line_for_register(eachline)

        elif eachline.startswith("CPUID"):
            cpuid_val = parse_line_for_register(eachline)

        elif eachline.startswith("MMFAR"):
            mmfar_val = parse_line_for_register(eachline)

        elif eachline.startswith("BFAR"):
            bfar_val = parse_line_for_register(eachline)

        else:
            for ram_str in ram_str_list:
                if ram_str not in adr_str_list and eachline.startswith("[" + ram_str + "]"):
                    ram4words = parse_line_for_ram(eachline)
                    ram_list =  ram4words.split()
                    idx_list = range(4)
                    ram_base = int(ram_str, 16)
                    adr_list = list(map(lambda a: "%08x" % (a * 4 + ram_base), idx_list))
                    adr_str_list += adr_list
                    val_str_list += ram_list
                    break

    print("\nCrash Info:")
    print("\tCrash location = %s [0x%s] (based on PC value)" % (pc_name.strip(), str(pc_val)))
    print("\tCaller location = %s [0x%s] (based on LR value)" % (lr_name.strip(), str(lr_val)))
    print("\tStack Pointer at the time of crash = [%s]" % (str(sp_val)))

    print("\tTarget and Fault Info:")
    print_CPUID_info(cpuid_val)
    print_HFSR_info(hfsr_val)
    print_MMFSR_info(mmfsr_val, mmfar_val)
    print_BFSR_info(bfsr_val, bfar_val)
    print_UFSR_info(ufsr_val)
    print_stack_call_trace(adr_str_list, val_str_list, elfhelper)

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Analyse mbed-os crash log. This tool requires arm-gcc binary utilities to be available in current path as it uses \'nm\' command')
    # specify arguments
    parser.add_argument(metavar='CRASH LOG', type=argparse.FileType('rb', 0),
                        dest='crashlog',help='path to crash log file')
    parser.add_argument(metavar='ELF FILE', type=argparse.FileType('rb', 0),
                        nargs='?',const=None,dest='elffile',help='path to elf file')
    parser.add_argument(metavar='MAP FILE', type=argparse.FileType('rb', 0),
                        nargs='?',const=None,dest='mapfile',help='path to map file')

    # get and validate arguments
    args = parser.parse_args()

    # if both the ELF and MAP files are present, the addresses can be converted to symbol names
    if args.elffile and args.mapfile:
        elfhelper = ElfHelper(args.elffile, args.mapfile)
    else:
        print("ELF or MAP file missing, logging raw values.")
        elfhelper = None

    # parse input and write to output
    main(args.crashlog, elfhelper)

    #close all files
    if args.elffile:
        args.elffile.close()
    if args.mapfile:
        args.mapfile.close()
    args.crashlog.close()

