@echo off

set Path=%cd%\bin\;%Path%
set build_dir=%cd%\build\

REM Open 4coder
start 4ed.exe

call setup_cl_x64.bat


