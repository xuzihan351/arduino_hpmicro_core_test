@echo off
rem Copyright 2024 hpmicro
rem SPDX-License-Identifier: BSD-3-Clause

setlocal enabledelayedexpansion
set ELF_FILE=%1%
set BUILD_TMP_DIR=%2%
set "NORMALIZED_FILE_NAME=%ELF_FILE:\=/%"

echo !BUILD_TMP_DIR!/download.jlink
echo erase > !BUILD_TMP_DIR!/download.jlink
echo loadfile !ELF_FILE!
echo loadfile !ELF_FILE! >> !BUILD_TMP_DIR!/download.jlink
echo r >> !BUILD_TMP_DIR!/download.jlink
echo go >> !BUILD_TMP_DIR!/download.jlink
echo q >> !BUILD_TMP_DIR!/download.jlink
JLink.exe -device HPM5361xCBx -if jtag -JTAGConf -1,-1 -autoconnect 1 -speed 4000 !BUILD_TMP_DIR!/download.jlink
