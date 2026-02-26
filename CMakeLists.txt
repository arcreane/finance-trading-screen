cmake_minimum_required(VERSION 3.16)

project(TradingLayoutSkeleton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt path BEFORE find_package
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.0/msvc2022_64")

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts Sql Network)

set(PROJECT_SOURCES
        src/main.cpp
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/ui/TickerPlaceholder.cpp
        src/ui/TickerPlaceholder.h
        src/ui/ChartWidget.cpp
        src/ui/ChartWidget.h
        src/core/orderbook.cpp
        src/core/orderbook.h
        src/ui/TradingBottomPanel.cpp
        src/ui/TradingBottomPanel.h
        src/ui/OrderEntryPanel.cpp
        src/ui/OrderEntryPanel.h
)

add_executable(TradingLayoutSkeleton ${PROJECT_SOURCES})

# Include directories for header resolution
target_include_directories(TradingLayoutSkeleton PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(TradingLayoutSkeleton PRIVATE Qt6::Widgets Qt6::Charts Qt6::Sql Qt6::Network)