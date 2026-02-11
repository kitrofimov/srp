add_custom_target(docs ALL)
add_custom_command(
	TARGET docs COMMAND doxygen -q
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
