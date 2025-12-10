cmake_minimum_required(VERSION 3.16)

project(TradingLayoutSkeleton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")

find_package(Qt6 COMPONENTS Core Gui Widgets Charts REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        TickerPlaceholder.cpp
        TickerPlaceholder.h
)

add_executable(TradingLayoutSkeleton ${PROJECT_SOURCES})

target_link_libraries(TradingLayoutSkeleton PRIVATE Qt6::Widgets)
