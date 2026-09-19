cmake_minimum_required(VERSION 3.23)

project(metal-cpp-sample)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_custom_target(shaderlib DEPENDS ${metallib_out})

find_library(APPKIT_LIB AppKit)
find_library(FOUNDATION_LIB Foundation)
find_library(METAL_LIB Metal)
find_library(METALKIT_LIB MetalKit)
find_library(QUARTZCORE_LIB QuartzCore)

add_executable(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} shaderlib)

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
    ${FOUNDATION_LIB}
    ${METAL_LIB}
    ${METALKIT_LIB}
    ${QUARTZCORE_LIB}
)
