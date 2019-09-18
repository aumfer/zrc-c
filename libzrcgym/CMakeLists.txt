add_library(libzrcgym SHARED
    include/env.h
    src/env.c
    include/gym.h
    src/gym.c
)

target_include_directories(libzrcgym PUBLIC include)
target_include_directories(libzrcgym PRIVATE src)

target_link_libraries(libzrcgym PUBLIC libzrchost)
target_link_libraries(libzrcgym PUBLIC libzrcdraw)
target_link_libraries(libzrcgym PUBLIC tinycthread)

#https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake/34445220
add_custom_command(TARGET libzrcgym POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_SOURCE_DIR}/tensorflow/lib/tensorflow.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:libzrcgym>)                 # <--this is out-file path