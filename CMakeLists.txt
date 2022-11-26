cmake_minimum_required(VERSION 3.22)
project(py3dengine C)

set(CMAKE_C_STANDARD 17)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/glad/include)
find_package(Python COMPONENTS Interpreter)
add_custom_command(
        OUTPUT lib/glad/include/glad/gl.h lib/glad/include/KHR/khrplatform.h lib/glad/src/gl.c
        COMMAND ${Python_EXECUTABLE} -m glad --out-path lib/glad --api gl:core c
)

add_executable(py3dengine src/main.c lib/glad/src/gl.c)
target_link_libraries(py3dengine glfw m)
