file(GLOB ComBombGui_SRCS
    "*.h"
    "*.cpp"
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CMAKE_CXX_FLAGS "-static-libstdc++")
endif()

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
#    add_library(X11             STATIC IMPORTED)
#    add_library(X11-xcb         STATIC IMPORTED)
#    add_library(xcb-render-util STATIC IMPORTED)
#    add_library(xcb-render      STATIC IMPORTED)
#    add_library(xcb             STATIC IMPORTED)
#    add_library(xcb-image       STATIC IMPORTED)
#    add_library(xcb-icccm       STATIC IMPORTED)
#    add_library(xcb-sync        STATIC IMPORTED)
#    add_library(xcb-xfixes      STATIC IMPORTED)
#    add_library(xcb-shm         STATIC IMPORTED)
#    add_library(xcb-randr       STATIC IMPORTED)
#    add_library(xcb-shape       STATIC IMPORTED)
#    add_library(xcb-keysyms     STATIC IMPORTED)
#    set_target_properties(X11             PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libX11.a")
#    set_target_properties(X11-xcb         PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libX11-xcb.a")
#    set_target_properties(xcb-render-util PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-render-util.a")
#    set_target_properties(xcb-render      PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-render.a")
#    set_target_properties(xcb             PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb.a")
#    set_target_properties(xcb-image       PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-image.a")
#    set_target_properties(xcb-icccm       PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-icccm.a")
#    set_target_properties(xcb-sync        PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-sync.a")
#    set_target_properties(xcb-xfixes      PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-xfixes.a")
#    set_target_properties(xcb-shm         PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-shm.a")
#    set_target_properties(xcb-randr       PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-randr.a")
#    set_target_properties(xcb-shape       PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-shape.a")
#    set_target_properties(xcb-keysyms     PROPERTIES IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libxcb-keysyms.a")



#    target_link_libraries(ComBombGui
#        X11
#        X11-xcb
#        xcb-render-util
#        xcb-render
#        xcb
#        xcb-image
#        xcb-icccm
#        xcb-sync
#        xcb-xfixes
#        xcb-shm
#        xcb-randr
#        xcb-shape
#        xcb-keysyms)
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
        INTERFACE_LINK_LIBRARIES "${linked_libs};${gen_expr}"
    )
    message(STATUS "widgets: ${linked_libs};${gen_expr}")
endfunction()
updateQtWidgetsLinks(qtpcre)
updateQtWidgetsLinks(qtharfbuzzng)

#target_link_libraries(ComBombGui Qt5::Widgets)

#foreach(plugin ${Qt5Widgets_PLUGINS})
#  get_target_property(_loc ${plugin} LOCATION)
#  message("WIDGETS: Plugin ${plugin} is at location ${_loc}")
#endforeach()
#
#foreach(plugin ${Qt5Core_PLUGINS})
#  get_target_property(_loc ${plugin} LOCATION)
#  message("CORE: Plugin ${plugin} is at location ${_loc}")
#endforeach()
#
#foreach(plugin ${Qt5Gui_PLUGINS})
#    get_target_property(_loc ${plugin} LOCATION)
#    target_link_libraries(ComBombGui ${_loc})
#    message("GUI: Plugin ${plugin} is at location ${_loc}")
#endforeach()

set_property(TARGET ComBombGui PROPERTY CXX_STANDARD 11)
