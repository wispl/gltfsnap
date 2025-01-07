#pragma once

#include "scene.h"

#include <fastgltf/types.hpp>
#include <glad/gl.h>

class Renderer {
public:
	Renderer(GLuint program);
	void update_scene(Scene& scene);
	void update();
	void render() const;
private:
	// gl buffers
	GLuint vertex_buffer, index_buffer, command_buffer;
	GLuint vao;

	// scene data
	bool scene_dirty = false;
	Scene curr_scene;
	Scene next_scene;
	fastgltf::math::fmat4x4 view_projection;

	// uniforms and ubo
	GLuint model_uniform;
	GLuint view_proj_uniform;
	GLuint material_ubo;
};
