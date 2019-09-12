add_library(libzrc
    include/moving_average.h
    src/moving_average.c
    src/physics.c
    include/zrc.h
    src/zrc.c
)

target_include_directories(libzrc PUBLIC include)
target_include_directories(libzrc PRIVATE src)

target_link_libraries(libzrc PUBLIC chipmunk_static)
target_link_libraries(libzrc PUBLIC handmade-math)
target_link_libraries(libzrc PUBLIC klib)
target_link_libraries(libzrc PUBLIC sokol)
