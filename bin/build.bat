@echo off

set location=%cd%
set me="%~dp0"

set custom_root=%cd%
set custom_bin=%custom_root%\bin
cd %location%

if NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (call "%custom_bin%\setup_cl_x64.bat")

set debug=/Zi
set release=/O2 /Zi

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX

