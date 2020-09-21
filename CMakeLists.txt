cmake_minimum_required(VERSION 3.0.0)
project(conway VERSION 0.1.0)

add_executable(conway main.c)
include_directories("C:\\Program Files (x86)\\SDL2-2.0.12\\include")

set(SDL2_LIBRARIES "C:\\Program Files (x86)\\SDL2-2.0.12\\lib\\x86\\SDL2.lib;C:\\Program Files (x86)\\SDL2-2.0.12\\lib\\x86\\SDL2main.lib")
target_link_libraries(conway ${SDL2_LIBRARIES})