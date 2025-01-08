#pragma once

#include "scene.h"

#include <fastgltf/types.hpp>
#include <glad/gl.h>

class Renderer {
public:
	Renderer(GLuint program);
	void update_scene(Scene& scene);
	void update_window(int new_width, int new_height);
	void update();
	void render() const;
	void loop();
private:
	// window data
	int width, height;

	// gl buffers
	GLuint vertex_buffer, index_buffer, command_buffer;
	GLuint vao;

	// scene data
	bool scene_dirty = false;
	Scene curr_scene;
	Scene next_scene;
	Camera camera;

	// uniforms and ubo
	GLuint model_uniform;
	GLuint view_proj_uniform;
	GLuint material_ubo;
};
