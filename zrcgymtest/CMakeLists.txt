add_executable(zrcgymtest
    src/main.c
)

target_link_libraries(zrcgymtest PUBLIC libzrcgym)

set_target_properties(zrcgymtest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake/34445220
add_custom_command(TARGET zrcgymtest POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${CMAKE_SOURCE_DIR}/tensorflow/lib/tensorflow.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:zrcgymtest>)                 # <--this is out-file path

add_custom_command(TARGET zrcgymtest POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "$<TARGET_FILE_DIR:libzrcgym>/libzrcgym.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:zrcgymtest>)                 # <--this is out-file path