@echo off
cls
del kill-current-process.exe
del hook.dll
gcc main.c -O1 -nostdlib -nostartfiles -Wl,-eexe_main -Wl,--subsystem,windows -lkernel32 -luser32 -o kill-current-process.exe
gcc main.c -O1 -shared -nostdlib -Wl,-edll_main  -lkernel32 -luser32 -o hook.dll

strip hook.dll
strip kill-current-process.exe
