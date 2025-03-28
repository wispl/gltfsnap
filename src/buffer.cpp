#include "buffer.h"

#include <iterator>

void MeshBuffer::bind_buffer(GLuint vao)
{
	glVertexArrayVertexBuffer(vao, 0, vertices.id(), 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vao, indices.id());
}

void MeshBuffer::delete_buffer()
{
	vertices.delete_buffer();
	indices.delete_buffer();
}

void MeshBuffer::add_mesh(LoadedGLTF& gltf)
{
	if (loaded_meshes.find(gltf.path) == loaded_meshes.end()) {
		Header vheader = vertices.allocate(gltf.vertices.size());
		vertices.update(vheader, gltf.vertices);

		Header iheader = indices.allocate(gltf.indices.size());
		indices.update(iheader, gltf.indices);

		loaded_meshes[gltf.path] = MeshAllocation(vheader, iheader);
	}
}

void MeshBuffer::remove_mesh(LoadedGLTF& gltf)
{
	auto search = loaded_meshes.find(gltf.path);
	if (search != loaded_meshes.end()) {
		MeshAllocation allocation = search->second;
		vertices.deallocate(allocation.vertex_header);
		indices.deallocate(allocation.index_header);
		loaded_meshes.erase(search);
	}
}

MeshAllocation MeshBuffer::get_header(LoadedGLTF& gltf)
{
	// TODO: error handling
	return loaded_meshes[gltf.path];
}

void CommandBuffer::bind_buffer()
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer);
}

void CommandBuffer::delete_buffer()
{
	glDeleteBuffers(1, &buffer);
}

void CommandBuffer::record_commands(std::vector<DrawCommand> new_commands)
{
	if (commands.capacity() < commands.size() + new_commands.size()) {
		needs_resize = true;
	}
	commands.insert(commands.end(),
			std::make_move_iterator(new_commands.begin()),
			std::make_move_iterator(new_commands.end()));
}

void CommandBuffer::delete_commands(std::size_t start, std::size_t end)
{
	commands.erase(commands.begin() + start, commands.begin() + start + end);
}

void CommandBuffer::clear_commands()
{
	commands.clear();
}

void CommandBuffer::upload_commands()
{
	if (needs_resize) {
		GLuint resized;
		glCreateBuffers(1, &resized);
		glNamedBufferStorage(resized, commands.capacity()*sizeof(DrawCommand), nullptr, GL_DYNAMIC_STORAGE_BIT);
		glDeleteBuffers(1, &buffer);
		buffer = resized;
		needs_resize = false;
	}
	glNamedBufferSubData(buffer, 0, commands.size() * sizeof(DrawCommand), commands.data());
}
