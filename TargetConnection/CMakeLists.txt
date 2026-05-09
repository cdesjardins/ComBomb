qt_add_library(TargetConnection STATIC
    TgtTelnetConnection.cpp
    TgtSerialConnection.cpp
    TgtProcessConnection.cpp
    TgtThread.cpp
    TgtCppsshConnection.cpp
)

target_include_directories(TargetConnection PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMBOMB_INSTALL_DIR}/include
    ${CMAKE_SOURCE_DIR}/../include
)

target_compile_definitions(TargetConnection PRIVATE CPPSSH_STATIC)

target_link_libraries(TargetConnection
    PUBLIC
        Qt6::Core
        Qt6::Widgets
    PRIVATE
        combomb::boost_system
)
