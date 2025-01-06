#include "shaders.h"
#include "gltf.h"
#include "scene.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// #include <stb_image_write.h>

#include <stdlib.h>
#include <stdio.h>

#include <iostream>

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << "\n";
}

static void opengl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		std::cerr << message << "\n";
	} else {
		std::cout << message << "\n";
	}
}

int main(int argc, char** argv)
{
	bool running = true;
	GLFWwindow* window;
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "objsnap", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(opengl_error_callback, nullptr);

	auto program = compile_program();
	glUseProgram(*program);

	// add "./" in front of the path
	auto file = std::string_view { + argv[1] };
	auto gltf = load_gltf(file);

	while (glfwWindowShouldClose(window) != GLFW_TRUE) {
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glfwSwapBuffers(window);
	}

	// glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	// Write image Y-flipped because OpenGL
	// stbi_write_png("offscreen.png",
	//                width, height, 4,
	//                buffer + (width * 4 * (height - 1)),
	//                -width * 4);

	glDeleteProgram(*program);
	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
