# Assure that needed utilities are built
if(NOT BUILD_EXAMPLES)
    add_library(objparser SHARED ${CMAKE_SOURCE_DIR}/examples/utility/objparser.c)
    target_include_directories(objparser PUBLIC ${CMAKE_SOURCE_DIR}/include)
endif()

set(REF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/references")
set(SCENE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scenes")
set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/out")
file(GLOB SCENE_SOURCES "${SCENE_DIR}/*/*.c")

# It's not file(COPY ...) because the latter does not run every time
# the file/directory contents change
add_custom_target(copy_res_folder_to_tests ALL)
add_custom_command(
	TARGET copy_res_folder_to_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/examples/res
		${CMAKE_CURRENT_BINARY_DIR}/res
)

add_library(save_fb SHARED utils/stb_image_write.c utils/save.c)
target_include_directories(save_fb PUBLIC utils/)
target_include_directories(save_fb PUBLIC ${CMAKE_SOURCE_DIR}/include)

foreach(SOURCE_FILE ${SCENE_SOURCES})
    # Get path relative to scenes/
    file(RELATIVE_PATH REL_PATH ${SCENE_DIR} ${SOURCE_FILE})
    get_filename_component(SCENE_NAME ${SOURCE_FILE} NAME_WE)
    get_filename_component(SCENE_SUBDIR ${REL_PATH} DIRECTORY)

    set(TARGET_NAME ${SCENE_SUBDIR}_${SCENE_NAME})
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE srp save_fb objparser)
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/examples/utility)

    # Make matching output subdirectory
    file(MAKE_DIRECTORY ${OUT_DIR}/${SCENE_SUBDIR})

    add_test(
        NAME ${TARGET_NAME}
        COMMAND python3
            ${CMAKE_CURRENT_SOURCE_DIR}/compare.py
            --exe $<TARGET_FILE:${TARGET_NAME}>
            --ref "${REF_DIR}/${SCENE_SUBDIR}/${SCENE_NAME}.png"
            --out "${OUT_DIR}/${SCENE_SUBDIR}/${SCENE_NAME}.png"
    )

endforeach()
