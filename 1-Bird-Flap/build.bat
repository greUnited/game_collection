@echo off

cl /Zi src\win32_main.c /Fe:bin\bird_flap.exe /Fo:bin\\ user32.lib gdi32.lib
