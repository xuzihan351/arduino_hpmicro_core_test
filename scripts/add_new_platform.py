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
import argparse

parser = argparse.ArgumentParser(description = "Generate Arduino Zip package and add its info to an already existed json file. So we can use one json file to provide multiple packages.")
parser.add_argument('--name',
                    type=str,
                    help='the name of the package. Ex:--name=\"HPMicro 5300 Series Boards\"')
parser.add_argument('--soc',
                    type=str,
                    help='the soc name of the package. Ex:--soc=HPM5361')
parser.add_argument('--boards',
                    type=str,
                    help='the boards of the package. Ex:--boards=hpm5300evk,hpm5301evklite')
parser.add_argument('--output',
                    type=str,
                    help='the output path of the package. Ex:--output=E:/example.zip')
parser.add_argument('--json_output',
                    type=str,
                    help='the output path of the already existed json file. Ex:--json_output=E:/example.json')
parser.add_argument('--version',
                    type=str,
                    help='the version. Ex:--version=1.5.0')
parser.add_argument('--input',
                    type=str,
                    help='the input directory path.')


def do_zip_compress(dir_name, zip_name):
    zip = zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED)
    for root, dir, files in os.walk(dir_name):
        for file in files:
            abs_path=os.path.join(root, file)
            print("HPMicro" + abs_path.replace(dir_name, ""))
            zip.write(abs_path, "HPMicro" + abs_path.replace(dir_name, ""))
    print("compress done, file path:\n")
    print(zip.filename)
    zip.close()

def check_dir(dir):
    if not os.path.exists(os.path.join(dir, "platform.txt")):
        print("Can not find platform.txt in input dir")
        return -1
    if not os.path.exists(os.path.join(dir, "boards.txt")):
        print("Can not find boards.txt in input dir")
        return -2
    if not os.path.exists(os.path.join(dir, "cores")):
        print("Can not find cores in input dir")
        return -3
    if not os.path.exists(args.json_output):
        print("Can not find json file")
        return -4
    return 0

def main(args):
    if not os.path.isdir(args.input):
        print("args.input must be dir!")
        return
    if not check_dir(args.input) == 0:
        return
    do_zip_compress(args.input, args.output)
    with open(args.output, "rb") as f:
        sha256sum=hashlib.sha256(f.read()).hexdigest()
        print("sha256sum="+sha256sum)
        with open(args.json_output, "r") as ff:
            config = json.load(ff)
            found = False
            for item in config["packages"][0]["platforms"]:
                if item.get("architecture") == args.soc:
                    found = True
                    print("!!!Found already existed info, replace it!")
                    item["version"]=args.version
                    item["architecture"]=args.soc
                    item["name"]=args.name
                    item["archiveFileName"]=os.path.basename(args.output)
                    item["checksum"]="SHA-256:" + sha256sum
                    item["size"]=os.path.getsize(args.output)
                    item["url"]="http://192.168.20.31/" + os.path.basename(args.output)
                    boards=args.boards.split(',')
                    print(boards)
                    i = 0
                    for board in boards:
                        obj = {}
                        item["boards"].append(obj)
                        item["boards"][i]["name"]=board
                        i = i + 1
                    print("checksum is " + item["checksum"])
                    break
            if not found:
                print("Uanble to find existed info, add a new one")
                obj = {}
                obj["version"]=args.version
                obj["architecture"]=args.soc
                obj["name"]=args.name
                obj["archiveFileName"]=os.path.basename(args.output)
                obj["checksum"]="SHA-256:" + sha256sum
                obj["size"]=os.path.getsize(args.output)
                obj["url"]="http://192.168.20.31/" + os.path.basename(args.output)
                obj["boards"] = []
                boards=args.boards.split(',')
                print(boards)
                i = 0
                for board in boards:
                    obj2 = {}
                    obj["boards"].append(obj2)
                    obj["boards"][i]["name"]=board
                    i = i + 1
                print("checksum is " + obj["checksum"])
                config["packages"][0]["platforms"].append(obj)
                print(config)
        with open(args.json_output, 'w+') as file2:
            print(config)
            json.dump(config, file2, ensure_ascii=False, indent=4)
    print("json file path: ", args.json_output)
    
if __name__ == "__main__":
    args = parser.parse_args()
    if (args.soc == None):
        print("!!!soc is not found!!!")
        sys.exit(-1)
    if (args.input == None):
        args.input = os.getcwd()
        print("!!!input is not found!!!Default to ", os.getcwd())
    if (args.boards == None):
        args.boards = "HPMTestBoard"
        print("!!!boards is not found, default to HPMTestBoard")
    
    if (args.name == None):
        args.name = "HPMicro " + args.soc
        print("!!!name is not found, default to HPMicro Arduino Boards")
    if (args.version == None):
        args.version = "1.0.0"
        print("!!!version is not found, default to 1.0.0")
    if (args.json_output == None):
        args.json_output = os.path.join(os.getcwd(), "output.json")
        print("!!!json_output is not found, default to " + os.path.join(os.getcwd(), "output.json"))
    if (args.output == None):
        args.output = os.path.join(os.getcwd(), "output.zip")
        print("!!!output path is not found, default to " + os.path.join(os.getcwd(), "output.zip"))
    if not args.output.endswith(".zip"):
        args.output = args.output + ".zip"
        print("args.output is not end with .zip, add .zip to tail: ", args.output)

    main(args)