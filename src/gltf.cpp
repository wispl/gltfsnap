#include "gltf.h"

#include <fastgltf/types.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <stb_image.h>

static GLsizei level_count(int width, int height)
{
	return static_cast<GLsizei>(1 + floor(log2(width > height ? width : height)));
}

// From fastgltf
static Texture load_texture(fastgltf::Asset& asset, fastgltf::Image& image)
{
	Texture texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);

	std::visit(fastgltf::visitor {
		[](auto& arg) {},
		[&](fastgltf::sources::URI& filePath) {
			assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
			assert(filePath.uri.isLocalPath()); // We're only capable of loading local files.

			int width, height, nrChannels;

			const std::string path(filePath.uri.path().begin(), filePath.uri.path().end()); // Thanks C++.
			unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
			glTextureStorage2D(texture.id, level_count(width, height), GL_RGBA8, width, height);
			glTextureSubImage2D(texture.id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		},
		[&](fastgltf::sources::Array& vector) {
			int width, height, nrChannels;
			unsigned char *data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data()), static_cast<int>(vector.bytes.size()), &width, &height, &nrChannels, 4);
			glTextureStorage2D(texture.id, level_count(width, height), GL_RGBA8, width, height);
			glTextureSubImage2D(texture.id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		},
		[&](fastgltf::sources::BufferView& view) {
			auto& buffer_view = asset.bufferViews[view.bufferViewIndex];
			auto& buffer = asset.buffers[buffer_view.bufferIndex];
			// We only care about VectorWithMime here, because we
			// specify LoadExternalBuffers, meaning all buffers are
			// already loaded into a vector.
			std::visit(fastgltf::visitor {
				[](auto& arg) {},
				[&](fastgltf::sources::Array& vector) {
					int width, height, nrChannels;
					unsigned char* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(vector.bytes.data() + buffer_view.byteOffset),
						 static_cast<int>(buffer_view.byteLength), &width, &height, &nrChannels, 4);
					glTextureStorage2D(texture.id, level_count(width, height), GL_RGBA8, width, height);
					glTextureSubImage2D(texture.id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
					stbi_image_free(data);
				}
			}, buffer.data);
	      }
	}, image.data);

	glGenerateTextureMipmap(texture.id);
	return texture;
}

static Material load_material(fastgltf::Material& material)
{
	return Material{ material.pbrData.baseColorFactor };
}

static Mesh load_mesh(fastgltf::Asset& asset, std::vector<Texture>& textures, fastgltf::Mesh& gltf_mesh)
{
	Mesh mesh;
	std::vector<Vertex> vertices;
	std::vector<std::uint32_t> indices;

	for (auto&& it : gltf_mesh.primitives) {
		Primitive primitive;
		size_t initial = vertices.size();

		// position
		auto* pos = it.findAttribute("POSITION");
		auto& pos_accessor = asset.accessors[pos->accessorIndex];
		vertices.resize(initial + pos_accessor.count);
		fastgltf::iterateAccessor<fastgltf::math::fvec3>(asset, pos_accessor, [&](fastgltf::math::fvec3 pos) {
			vertices.push_back(Vertex{ pos, fastgltf::math::fvec2() });
		});

		// uv
		auto* uv = it.findAttribute("TEXCOORD_0");
		if (uv != it.attributes.end()) {
			auto& uv_accessor = asset.accessors[uv->accessorIndex];
			fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, uv_accessor, [&](fastgltf::math::fvec2 uv, size_t index) {
				vertices[initial + index].uv = uv;
			});
		}


		// std::size_t texcoord_index = 0;

		// materials
		if (it.materialIndex.has_value()) {
			// get material uniform
			primitive.material_id = it.materialIndex.value() + 1; // adjust for default material

			// get texture
			auto& base_texture = asset.materials[it.materialIndex.value()].pbrData.baseColorTexture;
			if (base_texture.has_value()) {
				auto& texture = asset.textures[base_texture->textureIndex];
				primitive.albedo_texture = textures[texture.imageIndex.value()].id;

				// TODO: see if the below is needed, apparently texcoord_index may not always be 0
				// if (base_texture->transform && base_texture->transform->texCoordIndex.has_value()) {
				// 	texcoord_index = base_texture->transform->texCoordIndex.value();
				// } else {
				// 	texcoord_index = base_texture.->texCoordIndex;
				// }
			}
		} else {
			primitive.material_id = 0;
		}

		// indices
		auto& index_accessor = asset.accessors[it.indicesAccessor.value()];
		primitive.draw_command.instance_count = 1;
		primitive.draw_command.base_instance = 0;
		primitive.draw_command.base_vertex = initial;
		primitive.draw_command.first_index = indices.size();
		primitive.draw_command.count = static_cast<std::uint32_t>(index_accessor.count);
		fastgltf::iterateAccessor<std::uint32_t>(asset, index_accessor, [&](std::uint32_t idx) {
			indices.push_back(idx + initial);
		});

		mesh.primitives.push_back(primitive);

		glCreateBuffers(1, &mesh.vertex_buffer);
		glCreateBuffers(1, &mesh.index_buffer);
		// TODO: change these to static later on
		glNamedBufferStorage(mesh.vertex_buffer, sizeof(Vertex) * vertices.size(), &vertices[0], GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(mesh.index_buffer, sizeof(std::uint32_t) * indices.size(), &indices[0], GL_DYNAMIC_STORAGE_BIT);
	}

	return mesh;
}

LoadedGLTF load_gltf(std::filesystem::path path)
{
	LoadedGLTF loaded_gltf;
	constexpr auto extensions = fastgltf::Extensions::KHR_mesh_quantization
		| fastgltf::Extensions::KHR_texture_transform
		| fastgltf::Extensions::KHR_materials_variants;

	constexpr auto options = fastgltf::Options::DontRequireValidAssetMember
		| fastgltf::Options::AllowDouble
		| fastgltf::Options::LoadExternalBuffers
		| fastgltf::Options::LoadExternalImages
		| fastgltf::Options::GenerateMeshIndices;

	auto gltf = fastgltf::MappedGltfFile::FromPath(path);

	
	fastgltf::Parser parser(extensions);
        auto asset = std::move(parser.loadGltf(gltf.get(), path.parent_path(), options).get());
	for (auto& image : asset.images) {
		loaded_gltf.textures.push_back(load_texture(asset, image));
	}
	for (auto& material : asset.materials) {
		loaded_gltf.materials.push_back(load_material(material));
	}
	for (auto& mesh : asset.meshes) {
		loaded_gltf.meshes.push_back(load_mesh(asset, loaded_gltf.textures, mesh));
	}

	// TODO: material buffer
	
	return loaded_gltf;
}
