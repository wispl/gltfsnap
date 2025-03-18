#pragma once

#include <vector>

#include <glad/gl.h>

struct Header {
	std::size_t start;
	std::size_t size;

	bool operator==(const Header& other)
	{
		return (start == other.start) && (size == other.size);
	}
};

// https://www.khronos.org/opengl/wiki/Buffer_Object_Streaming#Persistent_mapped_streaming
// Could improve performance more, but is much more complicated to implement.

// A buffer interface which points to the GPU buffer. This keeps tracks of all
// allocations and deallocations. Call `allocate` to get a header chunk and
// `update` to insert or update data at a given chunk. Allocate does not insert
// the data but only allocates data for it.
template <typename T>
class Buffer {
public:
	Buffer(GLuint id) : buffer(id) {
		element_size = sizeof(T);
		glNamedBufferStorage(buffer, capacity*element_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
	}

	~Buffer() {
		glDeleteBuffers(1, &buffer);
	}

	Header allocate(size_t data_size);
	void deallocate(Header header);
	void update(Header header, std::vector<T> data);
private:
	GLuint buffer;
	size_t element_size;
	size_t size {0};
	size_t capacity {256};

	std::vector<Header> used_list;
	std::vector<Header> free_list;
};
