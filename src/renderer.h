#pragma once

#include "scene.h"
#include "buffer.h"
#include "gltf.h"

#include <fastgltf/types.hpp>
#include <glad/gl.h>

class Renderer {
public:
	Camera camera;

	Renderer(GLuint program);
	// TODO: move these into scene itself, and maybe use move semantics?
	void update_scene(Scene& scene);
	void add_node(Node node);
	void remove_node(Node node);

	void update_window(int new_width, int new_height);
	void update();
	void render() const;
	void loop();
private:
	// window data
	int width, height;

	// gl buffers
	MeshBuffer meshbuffer;
	CommandBuffer commandbuffer;
	GLuint vertex_buffer, index_buffer, command_buffer;
	GLuint vao;

	// scene data
	bool scene_dirty = false;
	Scene curr_scene;
	Scene next_scene;

	// uniforms and ubo
	GLuint model_uniform;
	GLuint view_proj_uniform;
	GLuint material_ubo;
};
