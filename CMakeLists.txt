cmake_minimum_required(VERSION 3.31)
project(Medium)

set(CMAKE_CXX_STANDARD 17)

# GLAD
add_library(glad STATIC vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# GLFW
add_subdirectory(vendor/glfw)

# My libs
add_subdirectory(vendor/graphite)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG_MODE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DRELEASE_MODE)
endif()

# EXECUTABLE
#add_executable(MediumTesting testing/test3d.cpp)
add_executable(MediumTesting testing/new.cpp)
target_include_directories(MediumTesting PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(MediumTesting PRIVATE glfw glad Graphite)

set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)

add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSET_DIR}
        $<TARGET_FILE_DIR:MediumTesting>/assets
)

add_dependencies(MediumTesting copy_assets)

# === Platform-specific system linking ===
if(APPLE)
    target_link_libraries(MediumTesting PRIVATE
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework QuartzCore"
    )
elseif(UNIX)
    target_link_libraries(MediumTesting PRIVATE
            GL X11 Xrandr Xi pthread dl
    )
elseif(WIN32)
    target_link_libraries(MediumTesting PRIVATE
            opengl32 gdi32 user32
    )
endif()

# ================ LIBRARY VERSION ================
add_library(Medium INTERFACE)
target_include_directories(Medium INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(Medium INTERFACE glfw glad Graphite)

# === Platform-specific system linking ===
if(APPLE)
    target_link_libraries(Medium INTERFACE
            "-framework Cocoa"
            "-framework OpenGL"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework QuartzCore"
    )
elseif(UNIX)
    target_link_libraries(Medium INTERFACE
            GL X11 Xrandr Xi pthread dl
    )
elseif(WIN32)
    target_link_libraries(Medium INTERFACE
            opengl32 gdi32 user32
    )
endif()