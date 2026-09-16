cmake_minimum_required(VERSION 3.23)

project(metal-cpp-sample)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(FOUNDATION_LIB Foundation)
find_library(METAL_LIB Metal)
find_library(QUARTZCORE_LIB QuartzCore)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/metal_impl.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE external/metal-cpp)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${FOUNDATION_LIB} ${METAL_LIB} ${QUARTZCORE_LIB}
)
