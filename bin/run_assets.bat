@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

call tool_build_assets.exe


popd
