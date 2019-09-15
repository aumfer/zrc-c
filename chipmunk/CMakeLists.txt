set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(Chipmunk2D)
target_include_directories(chipmunk_static PUBLIC Chipmunk2D/include)

target_compile_definitions(chipmunk_static PUBLIC CP_USE_DOUBLES=0)

if(WIN32)
    target_compile_options(chipmunk_static PRIVATE "/wd4244")
    target_compile_options(chipmunk_static PRIVATE "/wd4200")
    target_compile_options(chipmunk_static PRIVATE "/wd4307")
endif()
