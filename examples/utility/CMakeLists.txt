add_library(window SHARED window.c)
target_link_libraries(window PRIVATE SDL2)
target_include_directories(
	window
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)
