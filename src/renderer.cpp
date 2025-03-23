#include "renderer.h"

#include "gltf.h"
#include "buffer.h"

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

Renderer::Renderer(GLuint program)
{
	glUseProgram(program);
        glCreateVertexArrays(1, &vao);

	glEnableVertexArrayAttrib(vao, 0);
	glEnableVertexArrayAttrib(vao, 1);

	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));

	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribBinding(vao, 1, 0);

	glBindVertexArray(vao);

	GLuint buffers[3];
	glCreateBuffers(3, buffers);
	mesh_buffer = MeshBuffer(buffers[0], buffers[1]);
	command_buffer = CommandBuffer(buffers[2]);

	model_uniform = glGetUniformLocation(program, "model");
	view_proj_uniform = glGetUniformLocation(program, "view_proj");
	glCreateBuffers(1, &material_ubo);
	glNamedBufferStorage(material_ubo, static_cast<GLsizeiptr>(sizeof(Material)), nullptr, GL_DYNAMIC_STORAGE_BIT);

	camera.set_position(glm::vec3{ 0.f, 0.f, 0.1f });

	glEnable(GL_DEPTH_TEST);
}

void Renderer::update_scene(Scene scene)
{
	scene = std::move(scene);
	scene_dirty = true;
}

void Renderer::add_node(Node node)
{
	scene_dirty = true;
	scene.nodes.push_back(node);
	mesh_buffer.add_mesh(*node.gltf);
}

void Renderer::remove_node(Node node)
{
	scene_dirty = true;
	scene.nodes.erase(std::find(scene.nodes.begin(), scene.nodes.end(), node));
	mesh_buffer.remove_mesh(*node.gltf);
}

void Renderer::update_window(int new_width, int new_height)
{
	width = new_width;
	height = new_height;
	glViewport(0, 0, width, height);
}

void Renderer::update()
{
	// TODO: This is pretty expensive, could optimise later, or just leave it
	// since updating the scene in this application is pretty rare.
	camera.update();

	// Generate commands when the scene changes. This seems wasteful but
	// this is generally parallelized and the alternative of tracking when
	// a node is added or removed is not the best either.
	//
	// A command is generated for each primitive, in the future it might
	// make sense to generate commands for each meshnode in gltf so we
	// can just submit the whole command buffer for drawing instead of
	// indexing it by a command_idx like we are doing now.
	if (scene_dirty) {
		command_buffer.clear_commands();
		std::vector<DrawCommand> commands;
		for (const auto& node : scene.nodes) {
			auto& gltf = (*node.gltf);
			MeshAllocation allocation = mesh_buffer.get_header(gltf);
			commands.reserve(commands.size() + gltf.primitive_count);

			for (const auto& mesh : gltf.meshes) {
				for (const auto prim : mesh.primitives) {
					DrawCommand cmd = {
						.count = prim.index_count,
						.instance_count = 1,
						.first_index = prim.first_index + allocation.index_header.start,
						.base_vertex = prim.base_vertex + allocation.vertex_header.start,
						.base_instance = 0
					};
					commands.push_back(cmd);
				}
			}
		}
		command_buffer.record_commands(commands);
		scene_dirty = false;
	}
}

void Renderer::render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// bind global buffers
	mesh_buffer.bind_buffer(vao);
	command_buffer.bind_buffer();

	// bind material uniform buffer
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, material_ubo);

	// set camera uniforms
	auto view = camera.view_matrix();
	auto proj = glm::perspective(glm::radians(70.0f), (static_cast<float>(width) / height), 0.1f, 100.0f);
	auto view_proj = proj * view;
	glUniformMatrix4fv(view_proj_uniform, 1, GL_FALSE, &view_proj[0][0]);

	command_buffer.upload_commands();
	for (auto& node : scene.nodes) {
		auto gltf = *node.gltf;
		for (auto& meshnode : gltf.meshnodes) {
			auto transform = node.transform * meshnode.transform;
			glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &transform[0][0]);
			auto& mesh = gltf.meshes[meshnode.mesh_idx];

			for (auto& primitive : mesh.primitives) {
				auto& material = gltf.materials[primitive.material_idx];
				auto& texture = gltf.textures[primitive.texture_idx];

				glBindTextureUnit(0, texture.id);
				glNamedBufferSubData(material_ubo, 0, sizeof(Material), reinterpret_cast<const void*>(&material));

				glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, reinterpret_cast<const void*>(sizeof(DrawCommand) * primitive.command_idx));
			}
		}
	}
}

void Renderer::loop()
{
	update();
	render();
}
