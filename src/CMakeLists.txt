rock_library(rock-display
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(rock-display_bin Main.cpp
    DEPS rock-display)

