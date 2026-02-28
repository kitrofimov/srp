add_subdirectory(utility)

# It's not file(COPY ...) because the latter does not run every time
# the file/directory contents change
add_custom_target(copy_res_folder ALL)
add_custom_command(
	TARGET copy_res_folder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_LIST_DIR}/res
		${CMAKE_CURRENT_BINARY_DIR}/res
)

function(add_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_NAME}.c)
    target_link_libraries(${EXAMPLE_NAME} srp window framelimiter objparser)
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/utility)
endfunction()

add_example(01_colored_triangle)
add_example(02_spinning_triangle)
add_example(03_textured_cube)
add_example(04_utah_teapot)
