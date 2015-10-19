cmake_minimum_required(VERSION 3.1)
project(ComBomb CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../include")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/../install")
include(compilerflags)
include(combombpackages)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../install/include
    ${CMAKE_SOURCE_DIR}/../botan/install/include/botan-1.11)

add_subdirectory(ComBombGui)
add_subdirectory(QTerminal)
add_subdirectory(TargetConnection)

