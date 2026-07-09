#!/usr/bin/env python3
# coding=utf-8
# Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.

import argparse
import csv
import os
import re

# from dataclasses import dataclass


from kconfiglib import Kconfig, BOOL, TRISTATE


class Chdir:
    """便利的切换工作目录
    """

    def __init__(self, work_dir: str):
        self._old_dir = os.getcwd()
        self._work_dir = os.path.abspath(work_dir)

    def __enter__(self):
        os.chdir(self._work_dir)

    def __exit__(self, exc_type, exc_val, exc_tb):
        os.chdir(self._old_dir)


# @dataclass
# class IniMapping:
#     # menuconfig配置项
#     config_name:str
#
#     # ini键名
#     ini_key_name:str
#
#     # 备注
#     note :str"


class IniMapping:
    def __init__(self, config_name="", ini_key_name="", note=""):
        self.config_name = config_name
        self.ini_key_name = ini_key_name
        self.note = note


def config_to_ini(ini_mapping, Kconfig_configs, ini_file: str):
    with open(ini_file, 'r+', encoding='utf8') as f:
        content = f.read()
        for config_name, im in ini_mapping.items():
            if config_name not in Kconfig_configs:
                continue
            #  CONFIG_INI_HOST_GPIO设为-1时，避免被转义
            if config_name == 'CONFIG_INI_HOST_GPIO':
                content = re.sub(r'((?:^|\b)' + im.ini_key_name + '=)(\S+?)($|#|;|\s)',
                                 r'\g<1>' + Kconfig_configs[config_name] + r'\g<3>', content, flags=re.M)
            else:
                content = re.sub(r'((?:^|\b)' + im.ini_key_name + '=)(\S+?)($|#|;|\s)',
                                 r'\g<1>' + re.escape(Kconfig_configs[config_name]) + r'\g<3>', content, flags=re.M)
        f.seek(0)
        f.write(content)


def environ_config():
    '''配置menuconfig相关环境变量'''
    # 设置生成的配置项前缀
    os.environ["CONFIG_"] = ""


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Modify the configuration items in menuconfig to ini parameters")
    parser.add_argument('ini_map_file',
                        help="The mapping relationship file between menuconfig configuration and ini parameter name(csv file)")
    parser.add_argument('kconfig_file', help="Kconfig file")
    parser.add_argument('kconfig_config_file', help="Menuconfig config file to read")
    parser.add_argument('ini_file', help="ini file to write")

    args = parser.parse_args()

    kconfig_config_file = args.kconfig_config_file
    kconfig_file = args.kconfig_file

    # 解析映射关系
    ini_mapping = {}
    with open(args.ini_map_file, newline="", encoding="gb2312") as f:
        reader = csv.reader(f)
        for row in reader:
            if reader.line_num == 1:
                continue
            ini_mapping[row[0].strip()] = IniMapping(row[0].strip(), row[1].strip(), row[2].strip())

    # 解析menuconfig配置
    kconfig_configs = {}
    bool_map = {'y': '1', 'm': '-1', 'n': '0'}
    with Chdir(os.path.dirname(kconfig_file)):
        environ_config()
        kconf = Kconfig(filename=os.path.basename(kconfig_file))
        # kconf.warn = 0
        kconf.load_config(kconfig_config_file)
        for sym in kconf.unique_defined_syms:
            if sym.orig_type in (BOOL, TRISTATE):
                kconfig_configs[sym.name] = bool_map[sym.str_value]
            else:
                kconfig_configs[sym.name] = sym.str_value

    # 写入ini
    config_to_ini(ini_mapping, kconfig_configs, args.ini_file)
