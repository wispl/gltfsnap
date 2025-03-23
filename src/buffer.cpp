#include "buffer.h"

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


void CommandBuffer::record_command(DrawCommand command)
{
	if (commands.capacity() == commands.size()) {
		resized = true;
	}
	commands.push_back(command);
}

void CommandBuffer::delete_commands(std::size_t start, std::size_t end)
{
	commands.erase(commands.begin() + start, commands.begin() + start + end);
}

void CommandBuffer::clear_commands(std::size_t index)
{
	commands.clear();
}

void CommandBuffer::upload_commands()
{
	if (resized) {
		GLuint resized;
		glCreateBuffers(1, &resized);
		glNamedBufferStorage(resized, commands.capacity()*sizeof(DrawCommand), commands.data(), GL_DYNAMIC_STORAGE_BIT);
		glDeleteBuffers(1, &buffer);
		buffer = resized;
	}
	glNamedBufferSubData(buffer, 0, commands.size() * sizeof(DrawCommand), commands.data());
}
