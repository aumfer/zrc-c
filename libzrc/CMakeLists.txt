add_library(libzrc
    include/camera.h
    src/camera.c
    src/caster.c
    include/color.h
    src/color.c
    src/contact_damage.c
    src/flight.c
    include/guid.h
    src/guid.c
    src/life.c
    src/locomotion.c
    include/moving_average.h
    src/moving_average.c
    src/physics_controller.c
    src/physics.c
    src/seek.c
    src/sense.c
    include/timer.h
    src/timer.c
    src/ttl.c
    include/zmath.h
    src/zmath.c
    include/zrc.h
    src/zrc.c
)

target_include_directories(libzrc PUBLIC include)
target_include_directories(libzrc PRIVATE src)

target_link_libraries(libzrc PUBLIC chipmunk_static)
target_link_libraries(libzrc PUBLIC handmade-math)
target_link_libraries(libzrc PUBLIC klib)
target_link_libraries(libzrc PUBLIC sokol)
