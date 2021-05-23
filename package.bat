@echo off

cmake --build build --parallel 32 --config Release -- /nologo
if not %ERRORLEVEL% == 0 then exit /B 1
@rd /S /Q pkg
mkdir pkg
copy build\Release\texed.exe pkg
robocopy art pkg\art /E
butler push --if-changed "pkg" "zaklaus/texed:win-64"