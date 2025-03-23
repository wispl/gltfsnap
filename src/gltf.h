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

	// These are needed to generate draw commands.
	std::size_t base_vertex, first_index, index_count;

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
	std::vector<Vertex> vertices;
	std::vector<std::uint32_t> indices;

	std::vector<Texture> textures;
	std::vector<Material> materials;
	std::vector<Mesh> meshes;
	size_t primitive_count {0};

	std::vector<MeshNode> meshnodes;
};

LoadedGLTF load_gltf(std::filesystem::path path);
