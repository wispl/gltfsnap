#pragma once

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <filesystem>
#include <vector>
#include <string>

struct Texture {
	GLuint id;
};

// TODO: it might make more sense to store texture inside material
struct Material {
	glm::vec4 base_color;
	float metallic;
	float roughness;
};

// Texture and position coordinates, the data is interleaved.
struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
};

// A OpenGL struct which species a draw command for MultiDrawElements.
struct DrawCommand {
	std::uint32_t count;		// how many indices
	std::uint32_t instance_count;	// how many instances to draw
	std::uint32_t first_index;	// offset to the first indice
	std::uint32_t base_vertex;	// offset to the first vertex
	std::uint32_t base_instance;	// offset for when drawing multiple instances
};


struct Primitive {
	// We could just store the GLuint handle to the texture, but that would
	// be inconsistent with how materials is handled. So both these variables
	// are indices to the actual values in the material and texture arrays.
	std::size_t material_idx;
	std::size_t texture_idx;
	// Eventually we want to sort primitives based on something like
	// material so there is less binding. In that case we need the index to
	// the command since the order is no longer the same.
	std::size_t command_idx;

	// TODO: see if these are neccessary, it might be possible to always
	// assume drawing triangles and an uint32_t as the index type
	// GLenum primitive_type;
	// GLenum index_type;
};

struct Mesh {
	std::vector<Primitive> primitives;
};

struct MeshNode {
	glm::mat4 transform;
	std::size_t mesh_idx;
};

// Contains all information needed to render a GLTF. Meshes depend on materials
// which depend on textures.
struct LoadedGLTF {
	std::string path;
	std::vector<DrawCommand> commands;
	std::vector<Vertex> vertices;
	std::vector<std::uint32_t> indices;

	std::vector<Texture> textures;
	std::vector<Material> materials;
	std::vector<Mesh> meshes;

	std::vector<MeshNode> meshnodes;
};

LoadedGLTF load_gltf(std::filesystem::path path);
