@echo off
if exist build rmdir /s /q build
mkdir build
cd build
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.10.0/msvc2022_64 ..
cmake --build . --config Release
