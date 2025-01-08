#include "renderer.h"

#include "gltf.h"

template <typename T>
static void merge(std::vector<T>& a, std::vector<T>& b) {
	a.reserve(a.size() + b.size());
	a.insert(a.end(), b.begin(), b.end());
}

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

	model_uniform = glGetUniformLocation(program, "model");
	view_proj_uniform = glGetUniformLocation(program, "view_proj");
	glCreateBuffers(1, &material_ubo);
	glNamedBufferStorage(material_ubo, static_cast<GLsizeiptr>(sizeof(Material)), nullptr, GL_DYNAMIC_STORAGE_BIT);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::update_scene(Scene& scene)
{
	next_scene = std::move(scene);
	scene_dirty = true;
}

void Renderer::update()
{
	// TODO: This is pretty expensive, could optimise later, or just leave it
	// since updating the scene in this application is pretty rare.
	if (scene_dirty) {
		std::vector<DrawCommand> commands;
		std::vector<Vertex> vertices;
		std::vector<std::uint32_t> indices;

		curr_scene = std::move(next_scene);
		for (const auto& node : curr_scene.nodes) {
			merge(commands, node.gltf.commands);
			merge(vertices, node.gltf.vertices);
			merge(indices, node.gltf.indices);
		}
		// TODO: check if this is optimal, or use glBufferData instead
		if (vertex_buffer != GL_NONE) {
			glDeleteBuffers(1, &vertex_buffer);
			glDeleteBuffers(1, &index_buffer);
			glDeleteBuffers(1, &command_buffer);
		}
		glCreateBuffers(1, &vertex_buffer);
		glCreateBuffers(1, &index_buffer);
		glCreateBuffers(1, &command_buffer);

		glNamedBufferStorage(vertex_buffer, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(index_buffer, sizeof(std::uint32_t) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT);
		glNamedBufferStorage(command_buffer, sizeof(DrawCommand) * commands.size(), commands.data(), GL_DYNAMIC_STORAGE_BIT);
		scene_dirty = false;
	}
}

void Renderer::render() const
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// bind global mesh buffers
	glVertexArrayVertexBuffer(vao, 0, vertex_buffer, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vao, index_buffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, material_ubo);

	for (auto& node : curr_scene.nodes) {
		for (auto& meshnode : node.gltf.meshnodes) {
			glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &meshnode.transform[0][0]);
			auto& mesh = node.gltf.meshes[meshnode.mesh_idx];

			for (auto& primitive : mesh.primitives) {
				auto& material = node.gltf.materials[primitive.material_idx];
				auto& texture = node.gltf.textures[primitive.texture_idx];

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
