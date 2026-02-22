add_library(
    utils SHARED
    window.c
    framelimiter.c
    objparser.c
)
target_link_libraries(utils PUBLIC SDL2)
target_include_directories(utils PUBLIC ${CMAKE_SOURCE_DIR}/include)
