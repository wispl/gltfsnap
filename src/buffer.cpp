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

