cmake_minimum_required(VERSION 3.16)
project(Medify VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

set(EXEC_SRCS
    src/main.cpp
    src/MainWindow.cpp
    include/MainWindow.h
    src/ProcessManager.cpp
    include/ProcessManager.h
)

add_executable(Medify ${EXEC_SRCS})

target_include_directories(Medify PRIVATE include)
target_link_libraries(Medify PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
