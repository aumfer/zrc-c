add_executable(zrc
    src/main.c
)

target_include_directories(zrc PUBLIC include)
target_include_directories(zrc PRIVATE src)

target_link_libraries(zrc PUBLIC libzrc)
target_link_libraries(zrc PUBLIC libzrcdraw)
target_link_libraries(zrc PUBLIC libzrchost)
target_link_libraries(zrc PUBLIC tinycthread)

set_target_properties(zrc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake/34445220
add_custom_command(TARGET zrc POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_SOURCE_DIR}/tensorflow/lib/tensorflow.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:zrc>)                 # <--this is out-file path