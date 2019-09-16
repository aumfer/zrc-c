add_library(libzrcgym SHARED
    include/ai.h
    src/ai.c
    include/gym.h
    src/gym.c
)

target_include_directories(libzrcgym PUBLIC include)
target_include_directories(libzrcgym PRIVATE src)

target_link_libraries(libzrcgym PUBLIC libzrchost)
