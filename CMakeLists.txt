cmake_minimum_required(VERSION 3.16)
project(CarTwinViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2 路径（预编译的 MinGW 版）
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/sdl2")
set(SDL2_INCLUDE_DIR "${SDL2_DIR}/include/SDL2")
set(SDL2_LIBRARY "${SDL2_DIR}/lib/libSDL2main.a" "${SDL2_DIR}/lib/libSDL2.dll.a" "${SDL2_DIR}/lib/libSDL2_image.dll.a")

# 源文件
file(GLOB SOURCES "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SDL2_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${SDL2_DIR}/lib/libSDL2main.a"
    "${SDL2_DIR}/lib/libSDL2.dll.a"
    "${SDL2_DIR}/lib/libSDL2_image.dll.a"
    "-static-libgcc" "-static-libstdc++"
    "-mwindows"
    gdi32 imm32 ole32 oleaut32 version winmm setupapi
)

# 复制 DLL 到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DIR}/bin/SDL2.dll"
        "${SDL2_DIR}/bin/SDL2_image.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# 复制 assets 到输出目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)
