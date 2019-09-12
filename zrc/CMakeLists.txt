add_executable(zrc
    include/draw_visual.h
    src/draw_visual.c
    include/font.h
    src/font.c
    src/main.c
)

target_include_directories(zrc PUBLIC include)
target_include_directories(zrc PRIVATE src)

target_link_libraries(zrc PUBLIC libzrc)
target_link_libraries(zrc PUBLIC tinycthread)

set_target_properties(zrc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
