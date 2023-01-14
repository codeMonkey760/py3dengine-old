cmake_minimum_required(VERSION 3.22)
project(py3dengine C)

set(CMAKE_C_STANDARD 17)

find_package(json-c REQUIRED)

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
configure_file(src/resources/TestComponent.py ${CMAKE_CURRENT_BINARY_DIR}/resources/TestComponent.py COPYONLY)
configure_file(config.ini ${CMAKE_CURRENT_BINARY_DIR}/config.ini COPYONLY)
configure_file(default.json ${CMAKE_CURRENT_BINARY_DIR}/default.json COPYONLY)

add_executable(
    py3dengine
    lib/glad/src/gl.c
    src/source/main.c
    src/source/util.c
    src/source/logger.c
    src/source/engine.c
    src/source/custom_string.c
    src/source/game_object.c
    src/source/resource_manager.c
    src/source/scene_importer.c
    src/source/rendering_context.c
    src/source/config.c
    src/source/json_parser.c
    src/source/wfo_parser/wfo_parser.c
    src/source/wfo_parser/vertex_data_list.c
    src/source/wfo_parser/object_list.c
    src/source/resources/base_resource.c
    src/source/resources/model.c
    src/source/resources/shader.c
    src/source/resources/material.c
    src/source/resources/texture.c
    src/source/resources/python_script.c
    src/source/python/py3denginemodule.c
    src/source/python/python_util.c
    src/source/python/python_wrapper.c
    src/source/python/pycomponent.c
    src/source/components/rotation_component.c
    src/source/components/model_renderer_component.c
    src/source/components/base_component.c
    src/source/components/transform_component.c
    src/source/components/camera_component.c
    src/source/components/component_factory.c
)
target_include_directories(py3dengine PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(py3dengine glfw Python::Python json-c SOIL inih m)
