#pragma once

#include <fastgltf/types.hpp>

class Renderer {
	Renderer(GLuint program);
	void update();
	void render(Scene& scene) const;
private:
	GLuint vao;
	fastgltf::math::fmat4x4 view_projection;

	// vertex shader
	GLuint model_uniform;
	GLuint view_proj_uniform;

	// fragment shader
};
