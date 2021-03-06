@echo off
SET me=%~dp0

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

del /Q %cd%\assets

if not exist "%cd%\assets" mkdir %cd%\assets

copy %RootDir%\assets\* %cd%\assets

call tool_build_assets.exe


popd
