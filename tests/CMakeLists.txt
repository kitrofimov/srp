set(REF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/references")
set(SCENE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scenes")
set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/out")
file(GLOB SCENE_SOURCES "${SCENE_DIR}/*.c")

# It's not file(COPY ...) because the latter does not run every time
# the file/directory contents change
add_custom_target(copy_res_folder_to_tests ALL)
add_custom_command(
	TARGET copy_res_folder_to_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/examples/res
		${CMAKE_CURRENT_BINARY_DIR}/res
)

# Ensure the output directory exists
file(MAKE_DIRECTORY ${OUT_DIR})

add_library(save_fb SHARED utils/stb_image_write.c utils/save.c)
target_include_directories(save_fb PUBLIC utils/)
target_include_directories(save_fb PUBLIC ${CMAKE_SOURCE_DIR}/include)

foreach(SOURCE_FILE ${SCENE_SOURCES})
    get_filename_component(SCENE_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${SCENE_NAME} ${SOURCE_FILE})
    target_link_libraries(${SCENE_NAME} PRIVATE srp save_fb objparser)
    target_include_directories(${SCENE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/examples/utility)

    add_test(
        NAME ${SCENE_NAME}
        COMMAND python3
                ${CMAKE_CURRENT_SOURCE_DIR}/compare.py
                --exe $<TARGET_FILE:${SCENE_NAME}>
                --ref "${REF_DIR}/${SCENE_NAME}.png"
                --out "${OUT_DIR}/${SCENE_NAME}.png"
    )
endforeach()
