@echo off
if exist build rmdir /s /q build
mkdir build
cd build
set PATH=C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.10.1\mingw_64\bin;%PATH%
cmake -GNinja -DCMAKE_PREFIX_PATH=C:/Qt/6.10.1/mingw_64 ..
cmake --build .
