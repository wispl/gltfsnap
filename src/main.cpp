#include "actionset.h"
#include "gltf.h"
#include "input.h"
#include "renderer.h"
#include "scene.h"
#include "shaders.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>

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
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "objsnap", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(opengl_error_callback, nullptr);

	// add "./" in front of the path
	auto file = std::string_view { + argv[1] };
	auto gltf = load_gltf(file);

	auto program = compile_program();
	auto renderer = Renderer(*program);
	renderer.update_window(640, 480);

	input::ActionSet main(
		ActionSets::DEFAULT,
		std::vector{
			input::Action{ DefaultActions::FORWARD,	 GLFW_KEY_W, "Move Foward" },
			input::Action{ DefaultActions::BACKWARD, GLFW_KEY_S, "Move Backward" },
			input::Action{ DefaultActions::LEFT,	 GLFW_KEY_A, "Move Left" },
			input::Action{ DefaultActions::RIGHT, 	 GLFW_KEY_D, "Move Right" },
			input::Action{ DefaultActions::QUIT, 	 GLFW_KEY_Q, "Quit" }
		}
	);
	input::add_actionset(main);
	input::enable_actionset(ActionSets::DEFAULT);
	// Set userdata on the window to the renderer to access it during callbacks
	glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&renderer));

	auto resize_callback = [](GLFWwindow* window, int width, int height) {
		static_cast<Renderer*>(glfwGetWindowUserPointer(window))->update_window(width, height);
	};

	auto key_callback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		// Gets the last state of the key, good for detecting if it is held since
		// GLFW_REPEAT is for text input only and not reliable for this usecase.
		bool pressed_before = glfwGetKey(window, key) == GLFW_PRESS;
		input::process_button(key, (action != GLFW_RELEASE), pressed_before);
	};

	glfwSetFramebufferSizeCallback(window, resize_callback);
	glfwSetKeyCallback(window, key_callback);

	auto scene = Scene();
	Node node = { .gltf = gltf, .transform = glm::mat4(1.0f) };
	scene.nodes.push_back(node);

	renderer.update_scene(scene);

	// TODO: could be better
	float last_frame, curr_frame, delta_time = 0;
	while (glfwWindowShouldClose(window) != GLFW_TRUE) {
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		// TODO: decide if we want this in the renderer, deltatime also
		// has to be moved in that case
		input::update(delta_time);
		glfwPollEvents();
		renderer.loop();
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
