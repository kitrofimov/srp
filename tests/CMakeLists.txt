set(REF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/references")
set(SCENE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scenes")
set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/out")
file(GLOB SCENE_SOURCES "${SCENE_DIR}/*.c")

# Ensure the output directory exists
file(MAKE_DIRECTORY ${OUT_DIR})

add_library(test_utils SHARED utils/stb_image_write.c utils/save.c)
target_include_directories(test_utils PUBLIC utils/)
target_include_directories(test_utils PUBLIC ${CMAKE_SOURCE_DIR}/include)

foreach(SOURCE_FILE ${SCENE_SOURCES})
    get_filename_component(SCENE_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${SCENE_NAME} ${SOURCE_FILE})
    target_link_libraries(${SCENE_NAME} PRIVATE srp test_utils)

    add_test(
        NAME ${SCENE_NAME}
        COMMAND python3
                ${CMAKE_CURRENT_SOURCE_DIR}/compare.py
                --exe $<TARGET_FILE:${SCENE_NAME}>
                --ref "${REF_DIR}/${SCENE_NAME}.png"
                --out "${OUT_DIR}/${SCENE_NAME}.png"
    )
endforeach()
