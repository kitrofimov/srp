# Software Rendering Pipeline (SRP) library
# Licensed under GNU GPLv3

set(
	SOURCES
	core/context.c
	core/buffer.c
	core/framebuffer.c
	core/texture.c
	math/mat.c
	math/vec.c
	memory/alloc.c
	memory/arena.c
	pipeline/draw.c
	pipeline/primitive_assembly.c
	pipeline/topology.c
	pipeline/vertex_processing.c
	raster/triangle.c
	raster/line.c
	raster/point.c
	raster/fragment.c
	utils/stb_image.c
	utils/message_callback.c
	utils/type.c
)

add_library(srp STATIC ${SOURCES})
target_link_libraries(srp PUBLIC m)
target_include_directories(srp PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_include_directories(srp PRIVATE ${CMAKE_SOURCE_DIR}/lib)
target_include_directories(srp PRIVATE ${CMAKE_SOURCE_DIR}/src)
