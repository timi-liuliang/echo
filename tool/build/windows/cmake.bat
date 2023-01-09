echo off

:: start in current directory 
cd /D "%~dp0"

:: view cmake version
call cmake.exe -version

:: create dir
if not exist build mkdir build
cd build

:: generate visual studio solution
call cmake.exe -DECHO_EDITOR_MODE=FALSE -G"Visual Studio 16 2019" -A x64 ../

:: mklink data
cd /D "%~dp0"
if not exist .\bin\app\win64\Debug\data mklink /D .\bin\app\win64\Debug\data ..\Release\data

:: mklink sln
if not exist .\echo.sln mklink .\echo.sln build\echo.sln

:: delay close
timeout /t 10