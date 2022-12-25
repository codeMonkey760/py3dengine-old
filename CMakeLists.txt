cmake_minimum_required(VERSION 3.22)
project(py3dengine C)

set(CMAKE_C_STANDARD 17)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/lib/glad/include)
find_package(Python COMPONENTS Interpreter Development)
add_custom_command(
        OUTPUT lib/glad/include/glad/gl.h lib/glad/include/KHR/khrplatform.h lib/glad/src/gl.c
        COMMAND ${Python_EXECUTABLE} -m glad --out-path lib/glad --api gl:core c
)

include_directories(src/headers)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources)
configure_file(src/resources/solid_objs.obj ${CMAKE_CURRENT_BINARY_DIR}/resources/solid_objs.obj COPYONLY)
configure_file(src/resources/solid_objs.mtl ${CMAKE_CURRENT_BINARY_DIR}/resources/solid_objs.mtl COPYONLY)

add_executable(
    py3dengine
    src/source/main.c
    src/source/util.c
    lib/glad/src/gl.c
    src/source/shader.c
    src/source/quad.c
    src/source/camera.c
    src/source/logger.c
    src/source/engine.c
    src/source/custom_string.c
    src/source/game_object.c
    src/source/model.c
    src/source/wfo_parser/wfo_parser.c
    src/source/wfo_parser/vertex_data_list.c
    src/source/wfo_parser/object_list.c
    src/source/python_wrapper.c
    src/source/material.c
    src/source/resource_manager.c
        src/source/scene_importer.c src/headers/scene_importer.h)
target_include_directories(py3dengine PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(py3dengine glfw Python::Python m)
