add_library(libzrchost
    include/zrc_host.h
    src/zrc_host.c
)

target_include_directories(libzrchost PUBLIC include)
target_include_directories(libzrchost PRIVATE src)

target_link_libraries(libzrchost PUBLIC libzrc)
