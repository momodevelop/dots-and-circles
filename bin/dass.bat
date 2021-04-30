@echo off
    
SET me=%~dp0
    

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

call devenv tool_build_assets.exe


popd
