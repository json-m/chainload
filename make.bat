@echo off
gcc -Os -s -fno-ident -fno-asynchronous-unwind-tables -fno-stack-protector -fomit-frame-pointer -mwindows chainload.c -lkernel32 -luser32 -o chainload.exe