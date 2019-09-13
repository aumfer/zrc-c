add_library(libzrc
    include/camera.h
    src/camera.c
    include/color.h
    src/color.c
    src/flight.c
    src/life.c
    include/moving_average.h
    src/moving_average.c
    src/physics_controller.c
    src/physics.c
    include/timer.h
    src/timer.c
    include/zrc.h
    src/zrc.c
)

target_include_directories(libzrc PUBLIC include)
target_include_directories(libzrc PRIVATE src)

target_link_libraries(libzrc PUBLIC chipmunk_static)
target_link_libraries(libzrc PUBLIC handmade-math)
target_link_libraries(libzrc PUBLIC klib)
target_link_libraries(libzrc PUBLIC sokol)
