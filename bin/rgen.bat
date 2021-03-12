@echo off
SET me=%~dp0

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET CodeDir=%RootDir%\code

pushd %BuildDir%

if not exist "%CodeDir%\generated" mkdir %CodeDir%\generated

call tool_codegen.exe


popd
