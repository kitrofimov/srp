add_subdirectory(utility)

# It's not file(COPY ...) because the latter does not run every time
# the file/directory contents change
add_custom_target(copy_res_folder ALL)
add_custom_command(
	TARGET copy_res_folder
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_CURRENT_LIST_DIR}/res
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res
)

add_subdirectory(01_colored_triangle)
add_subdirectory(02_spinning_triangle)
add_subdirectory(03_spinning_textured_cube)
add_subdirectory(04_utah_teapot)
