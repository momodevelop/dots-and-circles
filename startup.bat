@echo off

set Path=%cd%\bin\;%Path%
set build_dir=%cd%\build\

start nvim-qt .

call setup_cl_x64.bat


