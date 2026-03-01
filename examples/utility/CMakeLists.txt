add_library(window SHARED window.c)
target_link_libraries(window PUBLIC SDL2)
target_include_directories(window PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(framelimiter SHARED framelimiter.c)
target_include_directories(framelimiter PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(objparser SHARED objparser.c)
target_include_directories(objparser PUBLIC ${CMAKE_SOURCE_DIR}/include)
