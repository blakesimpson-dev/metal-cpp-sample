cmake_minimum_required(VERSION 3.23)

project(metal-cpp-sample)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SHADER_SOURCE "${CMAKE_SOURCE_DIR}/shaders/basic.metal")
set(AIR_OUT "${CMAKE_BINARY_DIR}/basic.air")
set(METALLIB_OUT "${CMAKE_BINARY_DIR}/basic.metallib")

add_custom_command(
  OUTPUT ${AIR_OUT}
  COMMAND xcrun -sdk macosx metal -c ${SHADER_SOURCE} -o ${AIR_OUT}
  DEPENDS ${SHADER_SOURCE}
)
add_custom_command(
  OUTPUT ${METALLIB_OUT}
  COMMAND xcrun -sdk macosx metallib ${AIR_OUT} -o ${METALLIB_OUT}
  DEPENDS ${AIR_OUT}
)

add_custom_target(shader-library DEPENDS ${METALLIB_OUT})

find_library(FOUNDATION_LIB Foundation)
find_library(METAL_LIB Metal)
find_library(QUARTZCORE_LIB QuartzCore)

add_executable(${PROJECT_NAME})

add_dependencies(${PROJECT_NAME} shader-library)

target_sources(${PROJECT_NAME} PRIVATE src/main.cc src/metal_impl.cc)

target_include_directories(${PROJECT_NAME} PRIVATE external/metal-cpp)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${FOUNDATION_LIB} ${METAL_LIB} ${QUARTZCORE_LIB}
)
