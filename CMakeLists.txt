cmake_minimum_required(VERSION 3.16)
project(Puzzles VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        src/*.cpp
)
# Exclude GUI sources from the core library (the GUI executable is defined below)
list(FILTER SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/src/gui/.*\\.cpp")
list(REMOVE_ITEM SOURCES "${PROJECT_SOURCE_DIR}/src/main.cpp")

add_library(PuzzlesLib ${SOURCES})
target_include_directories(PuzzlesLib PUBLIC ${PROJECT_SOURCE_DIR}/include/puzzles)

find_package(Qt6 COMPONENTS Widgets PrintSupport QUIET)
if(Qt6_FOUND)
    set(QT_WIDGETS_LIB Qt6::Widgets)
    set(QT_PRINTSUPPORT_LIB Qt6::PrintSupport)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 COMPONENTS Widgets PrintSupport QUIET)
    if(Qt5_FOUND)
        set(QT_WIDGETS_LIB Qt5::Widgets)
        set(QT_PRINTSUPPORT_LIB Qt5::PrintSupport)
        set(QT_VERSION_MAJOR 5)
    endif()
endif()

if(NOT QT_WIDGETS_LIB)
    message(FATAL_ERROR "Qt5 or Qt6 with Widgets and PrintSupport is required to build Puzzles.")
endif()

add_executable(Puzzles
        src/main.cpp
        src/gui/MazeWindow.cpp
        src/gui/MazeWidget.cpp
        src/gui/MazeGame.cpp
        src/gui/CrosswordWidget.cpp
        src/gui/CrosswordGenerator.cpp
        src/gui/WordSearchWidget.cpp
        src/gui/WordSearchGenerator.cpp
        src/gui/SudokuWidget.cpp
        src/gui/SudokuGenerator.cpp
        src/gui/CryptogramWidget.cpp
        src/gui/CryptogramGenerator.cpp
        resources.qrc
)
target_link_libraries(Puzzles PRIVATE PuzzlesLib ${QT_WIDGETS_LIB} ${QT_PRINTSUPPORT_LIB})
target_include_directories(Puzzles PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/gui
)
target_compile_definitions(Puzzles PRIVATE QT_VERSION_MAJOR=${QT_VERSION_MAJOR})

if(MSVC)
    target_compile_options(Puzzles PRIVATE /W4 /permissive-)
else()
    target_compile_options(Puzzles PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2)
endif()

# Windows executable settings
if(WIN32)
    set_target_properties(Puzzles PROPERTIES WIN32_EXECUTABLE ON)
    # Set icon for Windows
    target_sources(Puzzles PRIVATE ${PROJECT_SOURCE_DIR}/icon/logo.ico)
endif()

# macOS app settings
if(APPLE)
    set_target_properties(Puzzles PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_BUNDLE_NAME "Puzzles"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_ICON_FILE "logo.icns"
        MACOSX_FRAMEWORK_IDENTIFIER "com.avs.puzzles"
    )
    
    # Copy icon to Resources
    set_source_files_properties(
        ${PROJECT_SOURCE_DIR}/icon/logo.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
endif()

# Install targets for AppImage/Linux
set(CMAKE_INSTALL_PREFIX /usr)

install(TARGETS Puzzles DESTINATION bin)
install(FILES ${PROJECT_SOURCE_DIR}/icon/logo.png DESTINATION share/pixmaps RENAME puzzles.png)
install(FILES ${PROJECT_SOURCE_DIR}/icon/logo.png DESTINATION share/icons/hicolor/256x256/apps RENAME puzzles.png)
install(FILES ${PROJECT_SOURCE_DIR}/puzzles.desktop DESTINATION share/applications)

