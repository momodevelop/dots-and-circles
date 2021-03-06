@echo off

IF "%1"=="" (SET Folder=test) ELSE (SET Folder=%1)

SET me=%~dp0

SET RootDir=%me%..
SET SandboxDir=%RootDir%\sandbox\%Folder%

call bin\setup_cl_x64.bat
IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

SET CommonCompilerFlags=-MT -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET CommonCompilerFlags=-DSLOW -DINTERNAL  %CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -opt:ref

pushd %SandboxDir%

cl %CommonCompilerFlags% test.cpp -link %CommonLinkerFlags%

echo Running...
call test.exe

popd
