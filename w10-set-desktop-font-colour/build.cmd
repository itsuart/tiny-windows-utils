rm *.exe

gcc main.c -D DESKTOP_COLOUR=0 -std=c99 -O1 -nostdlib -nostartfiles -Wl,-eexe_main -Wl,--subsystem,windows -lkernel32 -luser32 -o make-desktop-font-colour-white.exe
strip make-desktop-font-colour-white.exe

gcc main.c -D DESKTOP_COLOUR=255 -std=c99 -O1 -nostdlib -nostartfiles -Wl,-eexe_main -Wl,--subsystem,windows -lkernel32 -luser32 -o make-desktop-font-colour-black.exe
strip make-desktop-font-colour-black.exe
