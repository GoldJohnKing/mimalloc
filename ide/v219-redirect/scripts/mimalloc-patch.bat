@echo off

cd %~dp0

echo Backup arma3_x64.exe to arma3_x64.exe.bak

copy /y arma3_x64.exe arma3_x64.exe.bak

echo:
echo Apply mimalloc-redirect.dll patch to arma3_x64.exe

echo:
mimalloc-minject.exe -v -f -i --postfix=v219-redirect arma3_x64.exe

echo:
echo Check if patch was successful

echo:
mimalloc-minject.exe -l arma3_x64.exe

echo:
echo Patch was successful if you see:
echo.  0: mimalloc-redirect.dll
echo.  1: mimalloc-v219-redirect.dll
echo on the above lines.

echo:
pause
