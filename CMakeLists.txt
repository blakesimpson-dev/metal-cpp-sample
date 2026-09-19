cmake_minimum_required(VERSION 3.26)

project(metal-cpp-sample)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

file(GLOB shader_sources "${CMAKE_SOURCE_DIR}/shaders/*.metal")

set(air_out)
foreach(shader_source ${shader_sources})
  get_filename_component(shader_name ${shader_source} NAME_WE)
  set(out "${CMAKE_BINARY_DIR}/${shader_name}.air")
  add_custom_command(
    OUTPUT ${out}
    COMMAND xcrun -sdk macosx metal -c ${shader_source} -o ${out}
    DEPENDS ${shader_source}
  )
  list(APPEND air_out ${out})
endforeach()

set(metallib_out "${CMAKE_BINARY_DIR}/shaders.metallib")
add_custom_command(
  OUTPUT ${metallib_out}
  COMMAND xcrun -sdk macosx metallib ${air_out} -o ${metallib_out}
  DEPENDS ${air_out}
)

add_custom_target(shaders DEPENDS ${metallib_out})
add_custom_target(
  assets
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
)

find_library(APPKIT_LIB AppKit REQUIRED)
find_library(FOUNDATION_LIB Foundation REQUIRED)
find_library(METAL_LIB Metal REQUIRED)
find_library(METALKIT_LIB MetalKit REQUIRED)
find_library(QUARTZCORE_LIB QuartzCore REQUIRED)

add_subdirectory(external/simdjson EXCLUDE_FROM_ALL)

set(FASTGLTF_COMPILE_AS_CPP20 ON CACHE BOOL "" FORCE)
add_subdirectory(external/fastgltf EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} assets shaders)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
    src/application_delegate.cc
    src/executable_path.cc
    src/main.cc
    src/metal_impl.cc
    src/metal_renderer.cc
    src/primitive_scene.cc
    src/view_delegate.cc
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE external/metal-cpp external/metal-cpp-extensions
)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    ${APPKIT_LIB}
    fastgltf::fastgltf
    ${FOUNDATION_LIB}
    ${METAL_LIB}
    ${METALKIT_LIB}
    ${QUARTZCORE_LIB}
)
