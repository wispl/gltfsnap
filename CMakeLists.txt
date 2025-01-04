cmake_minimum_required(VERSION 3.24)

project(gltfsnap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Fetching dependencies...")
include(FetchContent)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Do not build tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Do not build docs")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Do not build examples")
set(GLFW_INSTALL OFF CACHE BOOL "Do not install")
FetchContent_Declare(
	glfw
	GIT_REPOSITORY  https://github.com/glfw/glfw.git
	GIT_TAG	        3.4
	GIT_SHALLOW     TRUE
	FIND_PACKAGE_ARGS NAMES glfw3
)
FetchContent_Declare(
	fastgltf
	GIT_REPOSITORY  https://github.com/spnda/fastgltf.git
	GIT_TAG	        v0.8.0
	GIT_SHALLOW     TRUE
	FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(glfw fastgltf)
message(STATUS "Finished fetching dependencies!")

add_executable(${PROJECT_NAME} "")
target_link_libraries(${PROJECT_NAME} glfw fastgltf)
target_include_directories(${PROJECT_NAME} PRIVATE extern)
add_subdirectory(extern)
add_subdirectory(src)
