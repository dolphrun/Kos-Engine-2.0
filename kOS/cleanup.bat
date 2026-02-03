@echo off
for %%d in (.vs x64 bin out CMakeFiles build debug release CMakeCache.txt test-results) do if exist "%%d" rmdir /Q /S "%%d"
for /d %%d in (*.dir *.vcxproj *.filters *.user) do if exist "%%d" rmdir /Q /S "%%d"
rem Delete contents of Resource folder (same path as this .bat)
if exist "%~dp0Resource" (
    rmdir /Q /S "%~dp0Resource"
    mkdir "%~dp0Resource"
)
echo Cleanup done!