cmake_minimum_required(VERSION 3.22)
project(py3dengine C)

set(CMAKE_C_STANDARD 17)

include(FetchContent)
include(FindOpenGL)

if (NOT PY3D_SOIL_LOCATION)
    set(PY3D_SOIL_LOCATION https://github.com/paralin/soil.git)
endif()
FetchContent_Declare(
    soil
    GIT_REPOSITORY ${PY3D_SOIL_LOCATION}
    GIT_TAG master
)
FetchContent_GetProperties(soil)
if(NOT soil_POPULATED)
    FetchContent_Populate(soil)
    add_subdirectory(${soil_SOURCE_DIR} ${soil_BINARY_DIR} EXCLUDE_FROM_ALL)
    target_link_libraries(
        soil
        ${OPENGL_LIBRARIES}
    )
endif()

find_package(Python COMPONENTS Development)
find_package(ODE REQUIRED)
find_package(glfw3 REQUIRED)
find_package(json-c REQUIRED)

include_directories(src/headers)

add_executable(
    py3dengine
    src/source/glad/gl.c
    src/source/main.c
    src/source/util.c
    src/source/logger.c
    src/source/engine.c
    src/source/custom_string.c
    src/source/custom_path.c
    src/source/config.c
    src/source/json_parser.c
    src/source/physics/collision.c
    src/source/physics/collision_state.c
    src/source/wfo_parser/wfo_parser.c
    src/source/wfo_parser/vertex_data_list.c
    src/source/wfo_parser/object_list.c
    src/source/resources/base_resource.c
    src/source/resources/model.c
    src/source/resources/shader.c
    src/source/resources/material.c
    src/source/resources/texture.c
    src/source/resources/python_script.c
    src/source/resources/sprite.c
    src/source/python/py3denginemodule.c
    src/source/python/py3dmathmodule.c
    src/source/python/py3dloggermodule.c
    src/source/python/python_util.c
    src/source/python/python_wrapper.c
    src/source/python/py3dcomponent.c
    src/source/python/py3dgameobject.c
    src/source/python/py3dmodelrenderer.c
    src/source/python/py3drenderingcontext.c
    src/source/python/py3dspriterenderer.c
    src/source/python/py3dinput.c
    src/source/python/py3dcontactpoint.c
    src/source/python/py3dcollisionevent.c
    src/source/python/py3dscene.c
    src/source/python/py3dresourcemanager.c
    src/source/python/py3dtextrenderer.c
    src/source/python/py3drigidbody.c
    src/source/math/vector3.c
    src/source/math/quaternion.c
    src/source/importers/texture.c
    src/source/importers/shader.c
    src/source/importers/component.c
    src/source/importers/scene.c
    src/source/importers/sprite_sheet.c
    src/source/importers/builtins.c
)

find_library(MATH_LIBRARY m)
if (MATH_LIBRARY)
    target_link_libraries(py3dengine ${MATH_LIBRARY})
endif()
target_include_directories(py3dengine PRIVATE ${soil_SOURCE_DIR}/inc)
target_include_directories(py3dengine PRIVATE ${Python_INCLUDE_DIRS})
target_link_libraries(py3dengine Python::Python json-c::json-c soil ODE::ODE glfw)

if (NOT PY3D_TEST_PROJECT_LOCATION)
    message(FATAL_ERROR, "Please set 'PY3D_TEST_PROJECT_LOCATION'")
endif()

add_custom_command(
    TARGET
    py3dengine
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PY3D_TEST_PROJECT_LOCATION} ${CMAKE_CURRENT_BINARY_DIR}
)
