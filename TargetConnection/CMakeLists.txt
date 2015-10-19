file(GLOB TargetConnection_SRCS
    "*.h"
    "*.cpp"
)
add_library(TargetConnection STATIC ${TargetConnection_SRCS})
qt5_use_modules(TargetConnection Widgets)
target_link_libraries(TargetConnection )

set_property(TARGET TargetConnection PROPERTY CXX_STANDARD 11)


