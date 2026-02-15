# Software Rendering Pipeline (SRP) library
# Licensed under GNU GPLv3

set(
	SOURCES
	context.c
	buffer.c
	triangle.c
	framebuffer.c
	type.c
	texture.c
	stb_image.c
	vec.c
	mat.c
	alloc.c
	message_callback.c
	arena.c
	draw.c
	primitive_assembly.c
	point.c
	fragment.c
	line.c
)

add_library(srp STATIC ${SOURCES})
target_link_libraries(srp PUBLIC m)
target_include_directories(
	srp PRIVATE
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_SOURCE_DIR}/include
)
