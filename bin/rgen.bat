@echo off
SET me=%~dp0

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%
SET TemplateDir=%RootDir%\code\templates
SET CodeDir=%RootDir%\code
pushd %BuildDir%

if not exist "%GeneratedDir%" mkdir %GeneratedDir

call tool_codegen.exe %TemplateDir%\array.tpl %CodeDir%\gen_tpl_array_char.h char Char


popd
