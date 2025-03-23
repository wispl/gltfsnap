#pragma once

#include "gltf.h"

#include <glad/gl.h>

#include <algorithm>
#include <unordered_map>
#include <vector>

/// Stores information on where an allocation starts and ends in terms of
/// lengths. When interacting with OpenGL, these values must be multiplied by
/// the element size because OpenGL expects them in terms of sizes rather than
/// lengths.
struct Header {
	std::size_t start;
	std::size_t size;

	bool operator==(const Header& other) {
		return (start == other.start) && (size == other.size);
	}
};

// https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming#Persistent_mapped_streaming
//
// Could improve performance more, but is much more complicated to implement.
// This allows for double or triple buffering which allows us to upload data to buffer
// while we are rendering, from multiple threads for example, which is not
// currently possible with this implementation.

/// A buffer interface which points to the GPU buffer. This keeps tracks of all
/// allocations and deallocations. Call `allocate` to get a header chunk and
/// `update` to insert or update data at a given chunk. Allocate does not insert
/// the data but only allocates data for it.
template <typename T>
class Buffer {
public:
	Buffer() {}
	Buffer(GLuint id) : buffer(id) {
		element_size = sizeof(T);
		glNamedBufferStorage(buffer, capacity*element_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	}
	~Buffer() { glDeleteBuffers(1, &buffer); }

	Header allocate(size_t data_size) {
		// Try using a free sector
		auto it = std::remove_if(free_list.begin(), free_list.end(), [data_size](Header h) { return h.size < data_size; });
		if (it != free_list.end()) {
			auto header = *it;
			auto start = header.start;
			auto mid = start + data_size;

			auto used_header = Header { .start = start, .size = data_size };
			auto free_header = Header { .start = mid, .size = header.size - data_size };

			used_list.push_back(used_header);
			// Keeps the free lsit ordered
			if (free_header.size != 0) {
				auto index = std::distance(free_list.begin(), it);
				free_list[index] = free_header;
			} else {
				free_list.erase(it);
			}

			return used_header;
		}

		// No free sectors, use the end and allocate some more space if needed
		bool needs_resize = false;
		while (capacity - size < data_size) {
			capacity *= 2;
			needs_resize = true;
		}
		if (needs_resize) {
			GLuint resized;
			glCreateBuffers(1, &resized);
			glNamedBufferStorage(resized, capacity*element_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
			glCopyNamedBufferSubData(buffer, resized, 0, 0, size * element_size);
			glDeleteBuffers(1, &buffer);
			buffer = resized;
		}

		auto used_header = Header { .start = size, .size = data_size };
		used_list.push_back(used_header);

		return used_header;
	};

	void deallocate(Header header) {
		auto used_header = std::find(used_list.begin(), used_list.end(), header);
		// TODO: This should probably error or panic otherwise.
		if (used_header != used_list.end()) {
			auto found = std::find_if(free_list.begin(), free_list.end(), [header](auto& h) { return header.start < h.start; });
			// No need to handle not found, since we just add it at the end anyways
			auto index = std::distance(free_list.begin(), found);

			// See if we can merge some sectors
			auto start = header.start;
			auto size = header.size;
			if (index > 0) {
				auto prev = free_list[index - 1];
				if (prev.start + prev.size == header.start) {
					start = prev.start;
					free_list.erase(free_list.begin() + index - 1);
					--index;
				}
			}
			if (index < size)  {
				auto next = free_list[index];
				if (next.start == header.start + header.size) {
					size = header.size + next.size;
					free_list.erase(free_list.begin() + index);
				}
			}
			free_list.insert(free_list.begin() + index, Header { .start = start, .size = size });
		}
		used_list.erase(used_header);
	}

	void update(Header header, std::vector<T> data) {
		glNamedBufferSubData(buffer, header.start * element_size, header.size * element_size, data.data());
	}
private:
	GLuint buffer;
	size_t element_size;
	size_t size {0};
	size_t capacity {256};

	std::vector<Header> used_list;
	std::vector<Header> free_list;
};

/// A MeshAllocation stores where the indices and vertices of a mesh is located
/// in the buffer.
struct MeshAllocation {
	MeshAllocation() {}
	MeshAllocation(Header vheader, Header iheader) : vertex_header(vheader), index_header(iheader) {}
	Header vertex_header, index_header;
};

/// Handles allocated meshes. Internally this is made up of vertices and
/// indicies of the mesh. For every mesh, a MeshAllocation is mapped which
/// stores where the data is located.
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

/// Stores DrawCommands to be uploaded to the GPU. This does not internally use
/// a Buffer because this must map up with nodes in Scene. In other words,
/// 	node 1, prim 1 will have command at 1
/// 	node 1, prim 2 will have command at 2
/// 	node 1, prim 3 will have command at 3
/// 	node 2, prim 1 will have command at 4
/// 	node 2, prim 2 will have command at 5
/// And so on. This keeps the rendering simple and negate the need for a
/// mapping of primitive to command, which would add more coupling between the
/// data and rendering code. In most cases, command buffers are regenerated
/// each frame rather than reused. Before drawing, upload_commands() must be
/// called.
class CommandBuffer {
public:
	CommandBuffer() {}
	CommandBuffer(GLuint id) : buffer(id) {
		commands.reserve(256);
		glNamedBufferStorage(buffer, 256*sizeof(DrawCommand), nullptr, GL_DYNAMIC_STORAGE_BIT);
	}
	~CommandBuffer() { glDeleteBuffers(1, &buffer); }

	// `commands` will be in an intdeterminate state and unusable after this.
	void record_commands(std::vector<DrawCommand> new_commands);
	void delete_commands(std::size_t start, std::size_t end);
	void clear_commands(std::size_t index);
	void upload_commands();
private:
	std::vector<DrawCommand> commands;
	size_t resized = false;
	GLuint buffer;
};
