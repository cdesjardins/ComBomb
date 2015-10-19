file(GLOB QTerminal_SRCS
    "*.h"
    "impl/*.h"
    "impl/*.cpp"
)

add_library(QTerminal STATIC ${QTerminal_SRCS})
qt5_use_modules(QTerminal Gui Widgets Core)
set_property(TARGET QTerminal PROPERTY CXX_STANDARD 11)
