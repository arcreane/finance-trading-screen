# Finance Trading Screen

A C++ Qt application for finance trading visualization.

## Prerequisites

- **Qt 6**: Ensure Qt 6 is installed (tested with 6.10.1).
- **MinGW**: Ensure a compatible MinGW compiler is available.
- **CMake**: Version 3.16 or higher.
- **Ninja**: Recommended build generator.

## Build and Run

I have provided batch scripts to simplify the process.

### Build
To build the project, run:
```cmd
build.bat
```
This script configures the project using CMake and Ninja, and builds the executable in the `build/` directory.

### Run
To run the application, execute:
```cmd
run.bat
```
This script sets up the necessary Qt environment variables and launches the application.

## Manual Build
If you prefer to build manually:

```cmd
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_PREFIX_PATH=C:/Qt/6.10.1/mingw_64 ..
cmake --build .
```
