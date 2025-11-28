cmake_minimum_required(VERSION 3.16)
project(finance-trading-screen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Sql)

add_executable(finance-trading-screen
    main.cpp
    MainWindow.cpp
    MainWindow.h
    ChartWidget.cpp
    ChartWidget.h
)

target_link_libraries(finance-trading-screen PRIVATE Qt6::Widgets Qt6::Charts Qt6::Sql)
