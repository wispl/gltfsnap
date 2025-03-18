#pragma once

#include "scene.h"
#include "buffer.h"
#include "gltf.h"

#include <fastgltf/types.hpp>
#include <glad/gl.h>

#include <unordered_map>

struct MeshAllocation {
	MeshAllocation() {}
	MeshAllocation(Header vheader, Header iheader) : vertex_header(vheader), index_header(iheader) {}
	Header vertex_header, index_header;
};

// Handles allocated meshes
class MeshBuffer {
public:
	MeshBuffer() {}
	MeshBuffer(GLuint vbo, GLuint ebo) : vertices(Buffer<Vertex>(vbo)), indices(Buffer<uint32_t>(ebo)) {}
	void add_mesh(LoadedGLTF& gltf);
	void remove_mesh(LoadedGLTF& gltf);
	MeshAllocation get_header(LoadedGLTF& gltf);
private:
	Buffer<Vertex> vertices;
	Buffer<uint32_t> indices;

	std::unordered_map<std::string, MeshAllocation> loaded_meshes;
};

class Renderer {
public:
	Camera camera;

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

	// uniforms and ubo
	GLuint model_uniform;
	GLuint view_proj_uniform;
	GLuint material_ubo;
};
