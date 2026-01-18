@echo off
echo Cleaning build directory...
if exist build rmdir /s /q build
mkdir build
cd build

echo Configuring with MinGW...
cmake -G "MinGW Makefiles" ..

if %errorlevel% equ 0 (
    echo Building...
    cmake --build .
) else (
    echo Configuration failed!
    pause
)
cd ..