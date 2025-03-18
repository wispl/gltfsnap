#include "buffer.h"

#include <algorithm>
#include <vector>

template <typename T>
Header Buffer<T>::allocate(size_t data_size)
{
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
}

template <typename T>
void Buffer<T>::deallocate(Header header)
{
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

template <typename T>
void Buffer<T>::update(Header header, std::vector<T> data)
{
	glNamedBufferSubData(buffer, header.start, header.size, data.data());
}
