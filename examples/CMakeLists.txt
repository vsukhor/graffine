set(execname create_merge_components)
set(appcpp main)

add_executable(${execname} ${execname}/${appcpp}.cpp)

target_link_libraries(${execname} PRIVATE graffine::graffine)

target_compile_definitions(${execname} PRIVATE
    # If the debug configuration pass the DEBUG define to the compiler
    $<$<CONFIG:Debug>: -DDEBUG>
)

set(bindir ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_custom_command(
    TARGET ${execname}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${execname}>  ${bindir}
)
