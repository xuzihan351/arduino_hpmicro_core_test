# Copyright (c) 2024 HPMicro
# SPDX-License-Identifier: BSD-3-Clause
#

import pdb
import jinja2
import sys
import argparse
import json
import os
import re
import zipfile
import hashlib
import shutil

SOC_MODULES_LIST_NAME = 'soc_modules.list'
def copy_source_files(target, source):
    for root, dir, files in os.walk(source):
        for dirr in dir:
            if not os.path.exists(os.path.join(target, os.path.relpath(os.path.join(root, dirr), source))):
                os.makedirs(os.path.join(target, os.path.relpath(os.path.join(root, dirr), source)))

        for file in files:
            print(file)
            print(os.path.splitext(file)[1])
            print(file, source, os.path.relpath(file, source), os.path.join(root, file))

            if os.path.splitext(file)[1] == ".c":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".h":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".cpp":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".hpp":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".s":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".S":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".md":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".MD":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".ld":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))
            if os.path.splitext(file)[1] == ".cfg":
                shutil.copy(os.path.join(root, file), os.path.join(target, os.path.relpath(os.path.join(root, file), source)))


def copy_driver_files(target, source, base, soc_series, soc_name):
    ip_list = []
    module_list = os.path.join(base, "soc", soc_series, soc_name, SOC_MODULES_LIST_NAME)
    with open(module_list, "r") as f:
        lines = f.readlines()
        for l in lines:
            if ip := re.match(r'HPMSOC_HAS_HPMSDK_(\w+)', l):
                ip_list.append(ip.group(1).lower())
    f.close()

    if not os.path.exists(os.path.join(target, "src")):
        os.makedirs(os.path.join(target, "src"))
    if not os.path.exists(os.path.join(target, "inc")):
        os.makedirs(os.path.join(target, "inc"))
    for ip in ip_list:
        driver_source=os.path.join(source, "src", "hpm_" + ip + "_drv.c")
        print(driver_source)
        if os.path.exists(driver_source):
            shutil.copy(driver_source, os.path.join(target, "src", "hpm_" + ip + "_drv.c"))
    for root, dir, files in os.walk(os.path.join(source, "inc")):
        for file in files:
            print(file)
            if os.path.splitext(file)[1] == ".h":
                shutil.copy(os.path.join(root, file), os.path.join(target, "inc"))

def main():
    # if (len(sys.argv)) < 4:
    #     print("arg1 is the abs path of target soc")
    #     print("arg2 is the abs path of output zip file path")
    #     print("arg3 is the abs path of output json file path")
    #     print("For Example: python3 E:\arduino\arduino_hpmicro\scripts\generate_zip.py E:\arduino\arduino_hpmicro\core\hpm6750 E:\arduino\HPMicro.zip E:\arduino\package_HPMicro_index2.json")
    #     return
    sdk_base = sys.argv[1]
    target_dir = sys.argv[2]
    board_name = sys.argv[3]
    soc_series = sys.argv[4]
    soc_name = sys.argv[5]
    # input_template = os.path.join(os.path.dirname(os.path.realpath(__file__)),"package_HPMicro_index.json")
    # out_json = sys.argv[3]
    # pdb.set_trace()
    if not os.path.isdir(sdk_base):
        print("HPM_SDK is not right!")
        return
    if not os.path.isdir(target_dir):
        os.makedirs(target_dir)
    if not os.path.exists(os.path.join(sdk_base, "boards")):
        print("Can't find dir boards in " + sdk_base)
        return
    else:
        target_root_dir = os.path.join(target_dir, "variants", board_name)
        if not os.path.isdir(target_root_dir):
            os.makedirs(target_root_dir)
        source_dir=os.path.join(sdk_base, "boards/" + board_name)
        copy_source_files(target_root_dir, source_dir)

    if not os.path.exists(os.path.join(sdk_base, "arch")):
        print("Can't find dir arch in " + sdk_base)
        return
    else:
        target_soc_root_dir = os.path.join(target_dir, "variants", board_name, soc_series, soc_name)
        if not os.path.isdir(target_soc_root_dir):
            os.makedirs(target_soc_root_dir)
        source_dir=os.path.join(sdk_base, "soc/" + soc_series + "/" + soc_name)
        copy_source_files(target_soc_root_dir, source_dir)
        target_ip_root_dir = os.path.join(target_dir, "variants", board_name, soc_series, "ip")
        if not os.path.isdir(target_ip_root_dir):
            os.makedirs(target_ip_root_dir)
        source_dir=os.path.join(sdk_base, "soc/" + soc_series + "/" + "ip")
        copy_source_files(target_ip_root_dir, source_dir)
        target_misc_root_dir = os.path.join(target_dir, "cores", "hpmicro", "misc")
        if not os.path.isdir(target_misc_root_dir):
            os.makedirs(target_misc_root_dir)
        source_dir=os.path.join(sdk_base, "utils")
        copy_source_files(target_misc_root_dir, source_dir)
        source_dir=os.path.join(sdk_base, "components\debug_console")
        copy_source_files(target_misc_root_dir, source_dir)
        target_driver_root_dir = os.path.join(target_dir, "variants", board_name, "drivers")
        if not os.path.isdir(target_driver_root_dir):
            os.makedirs(target_driver_root_dir)
        source_dir=os.path.join(sdk_base, "drivers")
        copy_driver_files(target_driver_root_dir, source_dir, sdk_base, soc_series, soc_name)
        target_arch_root_dir = os.path.join(target_dir, "cores", "hpmicro", "riscv")
        if not os.path.isdir(target_arch_root_dir):
            os.makedirs(target_arch_root_dir)
        source_dir=os.path.join(sdk_base, "arch/riscv")
        copy_source_files(target_arch_root_dir, source_dir)

    # do_zip_compress(input_dir, out_name)
    # with open(out_name, "rb") as f:
    #     sha256sum=hashlib.sha256(f.read()).hexdigest()
    #     print("sha256sum="+sha256sum)
    #     with open(input_template, "r") as ff:
    #         config = json.load(ff)
    #         config["packages"][0]["platforms"][0]["checksum"]="SHA-256:" + sha256sum
    #         config["packages"][0]["platforms"][0]["size"]=os.path.getsize(out_name)
    #         print(config["packages"][0]["platforms"][0]["checksum"])
    #         with open(out_json, 'w+') as file2:
    #             json.dump(config, file2)
    
    
if __name__ == "__main__":
    main()