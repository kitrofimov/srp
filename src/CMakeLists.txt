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
)

add_library(srp STATIC ${SOURCES})
target_link_libraries(srp PRIVATE m)
target_include_directories(
	srp PRIVATE
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_SOURCE_DIR}/include
)

