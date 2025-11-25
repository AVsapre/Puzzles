cmake_minimum_required(VERSION 3.16)
project(MazeUtil VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR}/include)
option(BUILD_CLI "Build the terminal version of MazeUtil" OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        src/*.cpp
)
list(FILTER SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/src/gui/.*\\.cpp")

list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main_cli.cpp")
add_library(MazeUtilLib ${SOURCES})
target_include_directories(MazeUtilLib PUBLIC ${PROJECT_SOURCE_DIR}/include/mazeutil)

find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(QT_WIDGETS_LIB Qt6::Widgets)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 COMPONENTS Widgets QUIET)
    if(Qt5_FOUND)
        set(QT_WIDGETS_LIB Qt5::Widgets)
        set(QT_VERSION_MAJOR 5)
    endif()
endif()

if(NOT QT_WIDGETS_LIB)
    message(FATAL_ERROR "Qt5 or Qt6 with Widgets is required to build MazeUtil.")
endif()

add_executable(MazeUtil
        src/main.cpp
        src/gui/MazeWindow.cpp
        src/gui/MazeWidget.cpp
        src/gui/MazeGame.cpp
)
target_link_libraries(MazeUtil PRIVATE MazeUtilLib ${QT_WIDGETS_LIB})
target_include_directories(MazeUtil PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/gui
)
target_compile_definitions(MazeUtil PRIVATE QT_VERSION_MAJOR=${QT_VERSION_MAJOR})

if(MSVC)
    target_compile_options(MazeUtil PRIVATE /W4 /permissive-)
else()
    target_compile_options(MazeUtil PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2)
endif()

if(BUILD_CLI)
    add_executable(MazeUtilCli src/main_cli.cpp)
    target_link_libraries(MazeUtilCli PRIVATE MazeUtilLib)
    target_include_directories(MazeUtilCli PRIVATE ${PROJECT_SOURCE_DIR}/include)
    if(MSVC)
        target_compile_options(MazeUtilCli PRIVATE /W4 /permissive-)
    else()
        target_compile_options(MazeUtilCli PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2)
    endif()
endif()
