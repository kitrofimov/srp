get_filename_component(NAME ${CMAKE_CURRENT_LIST_DIR} NAME)

add_executable(${NAME} main.c)
target_link_libraries(${NAME} srp SDL2 SDL2main window)
target_include_directories(
	${NAME}
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/../utility
)
