#include "shaders.h"
#include "gltf.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// #include <stb_image_write.h>
#include <linmath.h>

#include <stdlib.h>
#include <stdio.h>

#include <iostream>

static void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		std::cerr << message << '\n';
	} else {
		std::cout << message << '\n';
	}
}

int main(void) {
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	// glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(640, 480, "objsnap", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(glMessageCallback, nullptr);

	auto program = compile_program();



	// glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	// Write image Y-flipped because OpenGL
	// stbi_write_png("offscreen.png",
	//                width, height, 4,
	//                buffer + (width * 4 * (height - 1)),
	//                -width * 4);

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
