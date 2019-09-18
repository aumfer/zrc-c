add_library(libzrcdraw
    include/blur.h
    src/blur.c
    include/control.h
    src/control.c
    include/draw_blur.h
    src/draw_blur.c
    include/draw_locomotion.h
    src/draw_locomotion.c
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
    include/spines.h
    src/spines.c
    include/ui.h
    src/ui.c
    include/zrc_draw.h
    src/zrc_draw.c
)

target_include_directories(libzrcdraw PUBLIC include)
target_include_directories(libzrcdraw PRIVATE src)

target_link_libraries(libzrcdraw PUBLIC libzrc)
target_link_libraries(libzrcdraw PUBLIC par)
