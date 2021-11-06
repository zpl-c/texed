@echo off

cmake --build build_rel --parallel 32
if not %ERRORLEVEL% == 0 then exit /B 1
@rd /S /Q pkg
mkdir pkg
copy build_rel\texed.exe pkg
robocopy art pkg\art /E
butler push --if-changed "pkg" "zaklaus/texed:win-64"