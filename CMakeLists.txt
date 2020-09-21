cmake_minimum_required(VERSION 3.0.0)
project(conway VERSION 0.1.0)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
add_executable(conway main.c)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(SDL2_LIBRARIES "x86/SDL2main;x86/SDL2")
    set(BUILD_TYPE "x86")
else()
    set(SDL2_LIBRARIES "x64/SDL2main;x64/SDL2")
    set(BUILD_TYPE "x64")
endif()

target_link_libraries(conway ${SDL2_LIBRARIES})

add_custom_command(TARGET conway POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/${BUILD_TYPE}/SDL2.dll"
        $<TARGET_FILE_DIR:conway>)