@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

IF "%Opt%"=="assets" (
	REM call tool_build_assets.exe
	call tool_build_assets_v2.exe

) ELSE (
	call sdl_platform.exe
)
popd