qt_add_library(QTerminal STATIC
    impl/Vt102Emulation.cpp
    impl/TerminalView.cpp
    impl/TerminalModel.cpp
    impl/TerminalCharacterDecoder.cpp
    impl/SelfListener.cpp
    impl/ScreenWindow.cpp
    impl/Screen.cpp
    impl/QTerminalImpl.cpp
    impl/konsole_wcwidth.cpp
    impl/History.cpp
    impl/Emulation.cpp
    impl/KeyboardTranslator.cpp
    impl/TgtIntf.cpp
    impl/QTerminalConfig.cpp
    impl/CharacterColor.cpp

    # Header-only QObjects (or headers whose .cpp lives in a different
    # directory) — listed explicitly so AUTOMOC processes them.
    TgtIntf.h
    QTerminal.h
    QTerminalInterface.h
)

target_include_directories(QTerminal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMBOMB_INSTALL_DIR}/include
    ${CMAKE_SOURCE_DIR}/../include
)

target_link_libraries(QTerminal
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    PRIVATE
        combomb::boost_system
)
