add_library(window SHARED window.c framelimiter.c)
target_link_libraries(window PUBLIC SDL2)
target_include_directories(window PUBLIC ${CMAKE_SOURCE_DIR}/include)
