#include "gltf.h"

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <stb_image.h>

// Most of the code here is from fastgltf's gltf viewer example.

static GLsizei level_count(int width, int height)
{
	return static_cast<GLsizei>(1 + floor(log2(width > height ? width : height)));
}

static void load_texture(LoadedGLTF& gltf, fastgltf::Asset& asset, fastgltf::Image& image)
{
	Texture texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);

	std::visit(fastgltf::visitor {
		[](auto& arg) {},
		[&](fastgltf::sources::URI& filePath) {
			assert(filePath.fileByteOffset == 0);
			assert(filePath.uri.isLocalPath());

			int width, height, nrChannels;

			const std::string path(filePath.uri.path().begin(), filePath.uri.path().end());
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

	// TODO: samplers
	glGenerateTextureMipmap(texture.id);
	gltf.textures.push_back(texture);
}

static void load_material(LoadedGLTF& gltf, fastgltf::Material& material)
{
	gltf.materials.push_back(Material{
		glm::make_vec4(material.pbrData.baseColorFactor.data()),
		material.pbrData.metallicFactor,
		material.pbrData.roughnessFactor,
	});
}

static bool load_mesh(LoadedGLTF& gltf, fastgltf::Asset& asset, fastgltf::Mesh& gltf_mesh)
{
	Mesh mesh;
	for (auto&& it : gltf_mesh.primitives) {
		Primitive primitive;

		// position
		size_t vertices_start = gltf.vertices.size();
		auto* pos = it.findAttribute("POSITION");
		auto& pos_accessor = asset.accessors[pos->accessorIndex];
		fastgltf::iterateAccessor<glm::vec3>(asset, pos_accessor, [&](glm::vec3 pos) {
			gltf.vertices.push_back(Vertex{ pos, glm::vec2() });
		});
		size_t vertices_size = gltf.vertices.size() - vertices_start;

		// uv
		//
		// There might be more than one texture and thus more than one
		// texcoord, ignore that for now to keep it simple.
		auto* uv = it.findAttribute("TEXCOORD_0");
		if (uv != it.attributes.end()) {
			auto& uv_accessor = asset.accessors[uv->accessorIndex];
			fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, uv_accessor, [&](glm::vec2 uv, size_t index) {
				gltf.vertices[vertices_start + index].uv = uv;
			});
		}

		// materials and textures
		primitive.material_idx = 0;
		if (it.materialIndex.has_value()) {
			primitive.material_idx = it.materialIndex.value() + 1; // adjust for default material
			auto& base_texture = asset.materials[it.materialIndex.value()].pbrData.baseColorTexture;
			if (base_texture.has_value()) {
				auto& texture = asset.textures[base_texture->textureIndex];
				if (!texture.imageIndex.has_value()) {
					return false;
				}
				primitive.texture_idx = texture.imageIndex.value();

				// TODO: see if the below is needed, apparently texcoord_index may not always be 0
				// if (base_texture->transform && base_texture->transform->texCoordIndex.has_value()) {
				// 	texcoord_index = base_texture->transform->texCoordIndex.value();
				// } else {
				// 	texcoord_index = base_texture.->texCoordIndex;
				// }
			}
		}

		// indices
		auto& index_accessor = asset.accessors[it.indicesAccessor.value()];
		primitive.base_vertex = static_cast<std::size_t>(index_accessor.count);
		primitive.first_index = static_cast<std::size_t>(gltf.indices.size());
		primitive.index_count = static_cast<std::size_t>(vertices_start);
		primitive.command_idx = gltf.commands.size();
		DrawCommand cmd = {
			.count = static_cast<std::uint32_t>(index_accessor.count),
			.instance_count = 1,
			.first_index = static_cast<std::uint32_t>(gltf.indices.size()),
			.base_vertex = static_cast<std::uint32_t>(vertices_start),
			.base_instance = 0
		};
		gltf.commands.push_back(cmd);

		fastgltf::iterateAccessor<std::uint32_t>(asset, index_accessor, [&](std::uint32_t idx) {
			gltf.indices.push_back(idx);
		});

		mesh.primitives.push_back(primitive);
	}

	gltf.meshes.push_back(mesh);
	return true;
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

	// TODO: handle more than one scenes later
	assert(asset.scenes.size() == 1);

	for (auto& image : asset.images) {
		load_texture(loaded_gltf, asset, image);
	}

	loaded_gltf.path = path;
	// default material
	loaded_gltf.materials.push_back(Material{ glm::vec4(1.0f), 1.0f, 1.0f });
	for (auto& material : asset.materials) {
		load_material(loaded_gltf, material);
	}

	for (auto& mesh : asset.meshes) {
		load_mesh(loaded_gltf, asset, mesh);
	}

	fastgltf::iterateSceneNodes(asset, 0, fastgltf::math::fmat4x4(),
	    [&](fastgltf::Node& node, fastgltf::math::fmat4x4 transform) {
		    if (node.meshIndex.has_value()) {
			     loaded_gltf.meshnodes.push_back(MeshNode { glm::make_mat4(transform.data()), *node.meshIndex });
		    }
	});

	return loaded_gltf;
}
