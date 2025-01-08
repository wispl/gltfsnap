cmake_minimum_required(VERSION 3.24)

project(gltfsnap)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Resolving dependencies...")

include(FetchContent)

message(STATUS "Resolving glfw...")
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
message(STATUS "Resolved glfw!")

message(STATUS "Resolving glm...")
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG	        1.0.1
	GIT_SHALLOW     TRUE
	FIND_PACKAGE_ARGS
)
message(STATUS "Resolved glm!")

message(STATUS "Resolving fastgltf...")
FetchContent_Declare(
	fastgltf
	GIT_REPOSITORY  https://github.com/spnda/fastgltf.git
	GIT_TAG	        v0.8.0
	GIT_SHALLOW     TRUE
	FIND_PACKAGE_ARGS
)
message(STATUS "Resolved fastgltf!")

message(STATUS "Finished Resolving dependencies!")
FetchContent_MakeAvailable(glfw fastgltf glm)

add_executable(${PROJECT_NAME} "")
target_link_libraries(${PROJECT_NAME} PRIVATE glfw fastgltf glm)
target_include_directories(${PROJECT_NAME} PRIVATE extern)
add_subdirectory(extern)
add_subdirectory(src)
