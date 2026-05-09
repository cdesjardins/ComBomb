cmake_minimum_required(VERSION 3.21)
project(ComBomb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

set(COMBOMB_INSTALL_DIR  "${CMAKE_SOURCE_DIR}/../install"   CACHE PATH "Sibling install/ dir holding QueuePtr/cppssh/CDLogger")
set(COMBOMB_BOOST_DIR    "${CMAKE_SOURCE_DIR}/../external/boost/install" CACHE PATH "Boost install root")
set(COMBOMB_BOTAN_DIR    "${CMAKE_SOURCE_DIR}/../botan/install"          CACHE PATH "Botan install root")

# Imported static dependencies. The d-suffix variants live alongside the release
# variants; CMake selects per CMAKE_BUILD_TYPE.
function(combomb_import_static name release_path debug_path)
    add_library(${name} STATIC IMPORTED GLOBAL)
    set_target_properties(${name} PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE        "${release_path}"
        IMPORTED_LOCATION_RELWITHDEBINFO "${release_path}"
        IMPORTED_LOCATION_MINSIZEREL     "${release_path}"
        IMPORTED_LOCATION_DEBUG          "${debug_path}"
    )
endfunction()

combomb_import_static(combomb::QueuePtr
    "${COMBOMB_INSTALL_DIR}/lib/libQueuePtr.a"
    "${COMBOMB_INSTALL_DIR}/lib/libQueuePtrd.a")

combomb_import_static(combomb::cppssh
    "${COMBOMB_INSTALL_DIR}/lib/libcppssh.a"
    "${COMBOMB_INSTALL_DIR}/lib/libcppsshd.a")

combomb_import_static(combomb::CDLogger
    "${COMBOMB_INSTALL_DIR}/lib/libCDLogger.a"
    "${COMBOMB_INSTALL_DIR}/lib/libCDLoggerd.a")

combomb_import_static(combomb::botan
    "${COMBOMB_BOTAN_DIR}/lib/botan/release/libbotan-3.a"
    "${COMBOMB_BOTAN_DIR}/lib/botan/debug/libbotan-3.a")

add_library(combomb::boost_system STATIC IMPORTED GLOBAL)
set_target_properties(combomb::boost_system PROPERTIES
    IMPORTED_LOCATION "${COMBOMB_BOOST_DIR}/lib/libboost_system.a"
    INTERFACE_INCLUDE_DIRECTORIES "${COMBOMB_BOOST_DIR}/include"
    INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB;BOOST_NO_AUTO_PTR"
)

add_subdirectory(QTerminal)
add_subdirectory(TargetConnection)
add_subdirectory(ComBombGui)
