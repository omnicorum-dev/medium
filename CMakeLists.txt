cmake_minimum_required(VERSION 3.31)
project(Medium)

set(CMAKE_CXX_STANDARD 17)

add_library(glad STATIC vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

add_subdirectory(vendor/glfw)

add_executable(Medium testing/main.cpp)
target_include_directories(Medium PUBLIC vendor/glad/include vendor/glfw/include vendor/graphite vendor/graphite/vendor vendor/base-layer ./)
target_link_libraries(Medium PRIVATE glfw glad)

# === Platform-specific system linking ===
if(APPLE)
    target_link_libraries(Medium PRIVATE
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework QuartzCore"
    )
elseif(UNIX)
    target_link_libraries(Medium PRIVATE
            GL X11 Xrandr Xi pthread dl
    )
elseif(WIN32)
    target_link_libraries(Medium PRIVATE
            opengl32 gdi32 user32
    )
endif()
