
rem Copyright 2024 hpmicro
rem SPDX-License-Identifier: BSD-3-Clause

set BIN_FILE=%1%

set CMD_LINE=config-memory 0x10000 0x200" -r "write-memory 0x10000 0x80000400 %BIN_FILE%
D:\tools\hpm_manufacturing_cmd.exe -c -u 0x34b7,0x0005 -r "write-memory 0x0 0x200 [[0xfcf90002,0x07,0x100e,0x0]]" -r "%CMD_LINE%"
