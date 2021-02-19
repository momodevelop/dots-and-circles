@echo off

set Path=%cd%\bin\;%Path%
set build_dir=%cd%\build\

if not exist "%build_dir%" mkdir %build_dir%

call setup_cl_x64.bat


