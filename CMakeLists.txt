cmake_minimum_required(VERSION 3.10)

project(TrainGame VERSION 0.0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ENV{VULKAN_SDK} /home/cmfcmf/dev/train-game/vendor/1.2.141.2/x86_64)

# Generate config.hpp file from config.hpp file with #defines for the app version.
configure_file(config.hpp.in config.hpp)

file(GLOB SOURCES src/*.cpp)
add_executable(TrainGame ${SOURCES})

target_include_directories(TrainGame PUBLIC
                           "${PROJECT_BINARY_DIR}"
						  )

# glfw
# https://www.glfw.org/docs/3.3/build_guide.html
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw-3.3.2)
include_directories(vendor/glfw-3.3.2/include)
target_link_libraries(TrainGame glfw ${GLFW_LIBRARIES})

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(TrainGame Vulkan::Vulkan)

# Osmium
find_package(Osmium REQUIRED COMPONENTS pbf)
include_directories(${OSMIUM_INCLUDE_DIRS})

# ZLib (for Osmium)
target_link_libraries(TrainGame ZLIB::ZLIB)

# GeographicLib
add_subdirectory(vendor/GeographicLib-1.50.1)
include_directories(vendor/GeographicLib-1.50.1/include)
target_link_libraries(TrainGame GeographicLib)

# CityGML
find_package(CityGML REQUIRED)
target_link_libraries(TrainGame citygml)
include_directories(${CITYGML_INCLUDE_DIR})