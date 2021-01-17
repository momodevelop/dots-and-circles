@echo off

set Path=%cd%\bin\;%Path%
set build_dir=%cd%\build\

mkdir -p %build_dir%

pushd code
start momovim .
popd

call setup_cl_x64.bat


