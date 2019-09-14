add_executable(zrc
    include/blur.h
    src/blur.c
    include/control.h
    src/control.c
    include/draw_blur.h
    src/draw_blur.c
    include/draw_world.h
    src/draw_world.c
    include/draw_spines.h
    src/draw_spines.c
    include/draw_visual.h
    src/draw_visual.c
    include/draw.h
    src/draw.c
    include/font.h
    src/font.c
    src/main.c
    include/spines.h
    src/spines.c
    include/ui.h
    src/ui.c
)

target_include_directories(zrc PUBLIC include)
target_include_directories(zrc PRIVATE src)

target_link_libraries(zrc PUBLIC libzrc)
target_link_libraries(zrc PUBLIC par)
target_link_libraries(zrc PUBLIC tinycthread)

set_target_properties(zrc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
