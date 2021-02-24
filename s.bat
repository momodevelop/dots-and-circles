@echo off

set Path=%cd%\bin\;%Path%
set BuildDir=%cd%\build\

if not exist "%BuildDir%" mkdir %BuildDir%

call setup_cl_x64.bat
