@echo off

SET RootDir=%me%..
SET SandboxDir=%RootDir%\sandbox\%1

pushd %SandboxDir%

call devenv test.exe

:End 

popd
