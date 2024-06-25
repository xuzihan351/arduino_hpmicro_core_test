@echo off
rem Copyright 2024 hpmicro
rem SPDX-License-Identifier: BSD-3-Clause

setlocal enabledelayedexpansion
set OPENOCD_PATH=%1%
set ARG1=%2%
set ARG2=%3%
set ARG3=%4%
set ELF_FILE=%5%
set "NORMALIZED_FILE_NAME=%ELF_FILE:\=/%"
set OCD_CMD=program \"!NORMALIZED_FILE_NAME!\" verify reset exit
set OCD_CFG=-f !ARG1! -f !ARG2! -f !ARG3!
%OPENOCD_PATH% %OCD_CFG% -c "%OCD_CMD%"
