cmake_minimum_required(VERSION 3.0)
project(zrc)

if(WIN32)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/FAs)
    #add_compile_options(/GL)
    #add_link_options(/LTCG)
endif()

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(chipmunk)
add_subdirectory(handmade-math)
add_subdirectory(klib)
add_subdirectory(libzrc)
add_subdirectory(libzrcdraw)
add_subdirectory(libzrcgym)
add_subdirectory(libzrchost)
add_subdirectory(par)
add_subdirectory(sokol)
add_subdirectory(tinycthread)
add_subdirectory(zrc)

add_subdirectory(zrcgymtest)

#cmake .. -A x64 -T host=x64