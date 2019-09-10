add_library(libzrc
    src/physics.c
    include/zrc.h
)

target_include_directories(libzrc PUBLIC include)
target_include_directories(libzrc PRIVATE src)

target_link_libraries(libzrc PUBLIC chipmunk_static)
target_link_libraries(libzrc PUBLIC klib)

#target_compile_options(libzrc PRIVATE "/FAs")