find_package(Python3 REQUIRED COMPONENTS Interpreter)

# createVersion.py runs `git describe` and writes v.h next to versioning.cpp.
# The script writes to CWD-relative "v.h" and uses git from CWD, so we run
# it from the source dir — same as the original qmake build did.
set(VERSION_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/v.h)
add_custom_target(createVersion
    BYPRODUCTS ${VERSION_HEADER}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../createVersion.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating v.h"
    VERBATIM
)
set_source_files_properties(versioning.cpp PROPERTIES OBJECT_DEPENDS ${VERSION_HEADER})

qt_add_executable(ComBombGui
    main.cpp
    mainwindow.cpp
    childform.cpp
    opendialog.cpp
    aboutdialog.cpp
    versioning.cpp
    fileclipboarddialog.cpp
    configdialog.cpp
    runprocessdialog.cpp
    cbcombobox.cpp
    cblistwidget.cpp
    cbdialog.cpp
    finddialog.cpp
    cbfontcombobox.cpp
    capturedialog.cpp

    mainwindow.ui
    childform.ui
    opendialog.ui
    aboutdialog.ui
    fileclipboarddialog.ui
    configdialog.ui
    runprocessdialog.ui
    finddialog.ui
    capturedialog.ui

    ComBomb.qrc

    # Header-only QObjects (no matching .cpp) — listed so AUTOMOC processes them.
    cblabel.h
    fileclipboardheader.h
)

if(WIN32)
    target_sources(ComBombGui PRIVATE ComBomb.rc)
endif()

add_dependencies(ComBombGui createVersion)

target_include_directories(ComBombGui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}        # for cblabel.h / cbcombobox.h pulled in by .ui files, and v.h
    ${COMBOMB_INSTALL_DIR}/include
    ${CMAKE_SOURCE_DIR}/../include
)

target_compile_definitions(ComBombGui PRIVATE CPPSSH_STATIC)

target_link_libraries(ComBombGui PRIVATE
    QTerminal
    TargetConnection
    combomb::QueuePtr
    combomb::cppssh
    combomb::CDLogger
    combomb::botan
    combomb::boost_system
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if(UNIX AND NOT APPLE)
    target_link_options(ComBombGui PRIVATE -static-libstdc++)
endif()

# This Qt 6.8.3 install is static and does not bundle xcb. Import the
# platform plugins matching what the local Qt build provides.
qt_import_plugins(ComBombGui
    INCLUDE
        Qt6::QWaylandIntegrationPlugin
        Qt6::QMinimalIntegrationPlugin
        Qt6::QOffscreenIntegrationPlugin
)
