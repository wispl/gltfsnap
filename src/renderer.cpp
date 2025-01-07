#include "renderer.h"

#include "gltf.h"

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

	glEnable(GL_DEPTH_TEST);
}

void Renderer::render(Scene& scene) const
{
	for (auto& node : scene.nodes) {
		glUniformMatrix4fv(model_uniform, 1, GL_FALSE, &node.transform[0][0]);
		for (auto& mesh : node.gltf.meshes) {
			// bind mesh buffers
			glVertexArrayVertexBuffer(vao, 0, mesh.vertex_buffer, 0, sizeof(Vertex));
			glVertexArrayElementBuffer(vao, mesh.index_buffer);
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mesh.draw_buffer);

			// draw primitives
			for (auto& primitive : mesh.primitives) {
				glBindTextureUnit(0, primitive.albedo_texture);
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, mesh.materials[primitive.material_id]);
			}
		}
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT,
			      reinterpret_cast<const void*>(i * sizeof(Primitive)), 0);
	}
}
