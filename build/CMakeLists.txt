cmake_minimum_required(VERSION 3.20)
project(UsMInt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -std=c++20")

find_package(Freetype REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS} Vulkan::Headers)

add_library(UsMInt ../src/UI.h ../src/UI.cpp)

target_link_libraries(UsMInt Freetype::Freetype Vulkan::Vulkan)

install(TARGETS UsMInt
	LIBRARY DESTINATION /usr/local/lib)
install(FILES ../src/UI.h ../src/UI.cpp
	DESTINATION /usr/local/include/UsMInt)
# TODO: install as package
