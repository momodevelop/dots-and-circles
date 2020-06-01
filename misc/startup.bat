@echo off

cd ..

set Path=%cd%\bin\;%Path%
set build_dir=%cd%\_build\

mkdir %build_dir%

REM Open 4coder at code directory
pushd ..\code
start 4ed.exe
popd

call setup_cl_x64.bat

pu %build_dir%
