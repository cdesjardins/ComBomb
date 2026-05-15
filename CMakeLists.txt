cmake_minimum_required(VERSION 3.21)
project(ComBomb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

if(WIN32)
    # Boost headers (and the Windows headers Boost.Asio pulls in) need a
    # target Windows version. Win7 matches what MSVC defaults to anyway.
    add_compile_definitions(_WIN32_WINNT=0x0601)

    # Statically link the C/C++ runtime (/MT, /MTd) — matches the legacy qmake
    # build and the sibling repos when built via build.py. Must agree across
    # every translation unit linked into the executable, otherwise MSVC
    # raises LNK2038 ("mismatch detected for 'RuntimeLibrary'").
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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

# Convenience: ${P}<name>${X} resolves to "lib<name>.a" on Linux/macOS and
# "<name>.lib" on Windows — matching what the sibling repos and Botan install.
set(P "${CMAKE_STATIC_LIBRARY_PREFIX}")
set(X "${CMAKE_STATIC_LIBRARY_SUFFIX}")

combomb_import_static(combomb::QueuePtr
    "${COMBOMB_INSTALL_DIR}/lib/${P}QueuePtr${X}"
    "${COMBOMB_INSTALL_DIR}/lib/${P}QueuePtrd${X}")

combomb_import_static(combomb::cppssh
    "${COMBOMB_INSTALL_DIR}/lib/${P}cppssh${X}"
    "${COMBOMB_INSTALL_DIR}/lib/${P}cppsshd${X}")

combomb_import_static(combomb::CDLogger
    "${COMBOMB_INSTALL_DIR}/lib/${P}CDLogger${X}"
    "${COMBOMB_INSTALL_DIR}/lib/${P}CDLoggerd${X}")

combomb_import_static(combomb::botan
    "${COMBOMB_BOTAN_DIR}/lib/botan/release/${P}botan-3${X}"
    "${COMBOMB_BOTAN_DIR}/lib/botan/debug/${P}botan-3${X}")

# Boost.System became header-only in Boost 1.89, and the headers-only install
# of 1.91 ships no compiled libraries — Asio (the only Boost we use) builds
# header-only against it. The target name is kept for compatibility with the
# subdirectories that already link to it.
add_library(combomb::boost_system INTERFACE IMPORTED GLOBAL)
set_target_properties(combomb::boost_system PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${COMBOMB_BOOST_DIR}/include"
    INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB;BOOST_NO_AUTO_PTR"
)

add_subdirectory(QTerminal)
add_subdirectory(TargetConnection)
add_subdirectory(ComBombGui)
