cmake_minimum_required(VERSION 3.22)
project(py3dengine C)

set(CMAKE_C_STANDARD 17)
set(BUILD_TESTING OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)
FetchContent_GetProperties(glfw)
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
    json-c
    GIT_REPOSITORY https://github.com/json-c/json-c
    GIT_TAG master
)
if(NOT json-c_POPULATED)
    FetchContent_Populate(json-c)
    add_subdirectory(${json-c_SOURCE_DIR} ${json-c_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG master
)
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include(FindOpenGL)

FetchContent_Declare(
    soil
    GIT_REPOSITORY https://github.com/paralin/soil.git
    GIT_TAG master
)
FetchContent_GetProperties(soil)
if(NOT soil_POPULATED)
    FetchContent_Populate(soil)
    add_subdirectory(${soil_SOURCE_DIR} ${soil_BINARY_DIR} EXCLUDE_FROM_ALL)
    target_link_libraries(
        soil
        glad
        ${OPENGL_LIBRARIES}
    )
endif()

find_package(Python COMPONENTS Development)

include_directories(src/headers)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources)
configure_file(src/resources/solid_objs.obj ${CMAKE_CURRENT_BINARY_DIR}/resources/solid_objs.obj COPYONLY)
configure_file(src/resources/solid_objs.mtl ${CMAKE_CURRENT_BINARY_DIR}/resources/solid_objs.mtl COPYONLY)
configure_file(src/resources/RotationComponent.py ${CMAKE_CURRENT_BINARY_DIR}/resources/RotationComponent.py COPYONLY)
configure_file(src/resources/CameraComponent.py ${CMAKE_CURRENT_BINARY_DIR}/resources/CameraComponent.py COPYONLY)
configure_file(config.json ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)
configure_file(default.json ${CMAKE_CURRENT_BINARY_DIR}/default.json COPYONLY)

add_executable(
    py3dengine
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
    src/source/python/py3dmathmodule.c
    src/source/python/py3dloggermodule.c
    src/source/python/python_util.c
    src/source/python/python_wrapper.c
    src/source/python/py3dcomponent.c
    src/source/python/py3dtransform.c
    src/source/python/py3dmodelrenderer.c
    src/source/math/vector3.c
    src/source/math/quaternion.c
)

find_library(MATH_LIBRARY m)
if (MATH_LIBRARY)
    target_link_libraries(py3dengine ${MATH_LIBRARY})
endif()
target_include_directories(py3dengine PRIVATE ${json-c_SOURCE_DIR})
target_include_directories(py3dengine PRIVATE ${soil_SOURCE_DIR}/inc)
target_include_directories(py3dengine PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(py3dengine glfw glad Python::Python json-c soil)
