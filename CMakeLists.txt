cmake_minimum_required(VERSION 3.16)

project(finance-trading-screen VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    TradingBottomPanel.cpp
    TradingBottomPanel.h
)

# Create executable
add_executable(finance-trading-screen ${PROJECT_SOURCES})

# Link Qt libraries
target_link_libraries(finance-trading-screen PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Windows specific settings
if(WIN32)
    set_target_properties(finance-trading-screen PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
