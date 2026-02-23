cmake_minimum_required(VERSION 3.10)
project(GlorysTriumph LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Raylib package
find_package(raylib CONFIG REQUIRED)

# Include directories
include_directories(src)

# Source files
file(GLOB_RECURSE GAME_SOURCES "src/*.cpp")

# Add the executable
add_executable(GlorysTriumph ${GAME_SOURCES})

# Link Raylib libraries
target_link_libraries(GlorysTriumph PRIVATE raylib)

# Optionally, copy assets
# install(DIRECTORY resources DESTINATION .)
