file(GLOB ComBombGui_SRCS
    "*.h"
    "*.cpp"
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
add_executable(ComBombGui ${ComBombGui_SRCS})
target_link_libraries(ComBombGui QTerminal TargetConnection)
if (HAVE_CDLOGGER_LIB)
    target_link_libraries(ComBombGui optimized ${HAVE_CDLOGGER_LIB})
endif()
if (HAVE_CDLOGGER_DBG_LIB)
    target_link_libraries(ComBombGui debug ${HAVE_CDLOGGER_DBG_LIB})
endif()
if (HAVE_CPPSSH_LIB)
    target_link_libraries(ComBombGui optimized ${HAVE_CPPSSH_LIB})
endif()
if (HAVE_CPPSSH_DBG_LIB)
    target_link_libraries(ComBombGui debug ${HAVE_CPPSSH_DBG_LIB})
endif()
if (HAVE_BOTAN_LIB)
    target_link_libraries(ComBombGui optimized ${HAVE_BOTAN_LIB})
endif()
if (HAVE_BOTAN_DBG_LIB)
    target_link_libraries(ComBombGui debug ${HAVE_BOTAN_DBG_LIB})
endif()
if (HAVE_BOOST_SYSTEM_LIB)
    target_link_libraries(ComBombGui optimized ${HAVE_BOOST_SYSTEM_LIB})
endif()
if (HAVE_BOOST_SYSTEM_DBG_LIB)
    target_link_libraries(ComBombGui debug ${HAVE_BOOST_SYSTEM_DBG_LIB})
endif()
if (HAVE_QUEUEPTR_LIB)
    target_link_libraries(ComBombGui optimized ${HAVE_QUEUEPTR_LIB})
endif()
if (HAVE_QUEUEPTR_DBG_LIB)
    target_link_libraries(ComBombGui debug ${HAVE_QUEUEPTR_DBG_LIB})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(ComBombGui rt pthread z dl)
endif()


####
####
function(updateQtWidgetsLinks libbaseName)
    get_target_property(
        linked_libs
        Qt5::Widgets
        INTERFACE_LINK_LIBRARIES
    )

    set(debug "${_qt5Widgets_install_prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${libbaseName}d${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set(release "${_qt5Widgets_install_prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${libbaseName}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    set(debug_gen_expr "$<$<CONFIG:Debug>:${debug}>")
    set(release_gen_expr "$<$<NOT:$<CONFIG:Debug>>:${release}>")
    set(gen_expr "${debug_gen_expr};${release_gen_expr}")

    set_target_properties(
        Qt5::Widgets
        PROPERTIES
        INTERFACE_LINK_LIBRARIES "${gen_expr};${linked_libs}"
    )
endfunction()
updateQtWidgetsLinks(qtpcre)
updateQtWidgetsLinks(qtharfbuzzng)

target_link_libraries(ComBombGui Qt5::Widgets)
foreach(plugin ${Qt5Gui_PLUGINS})
    get_target_property(_loc ${plugin} LOCATION)
    target_link_libraries(ComBombGui ${_loc})
    message(STATUS ${_loc})
endforeach()

set_property(TARGET ComBombGui PROPERTY CXX_STANDARD 11)
