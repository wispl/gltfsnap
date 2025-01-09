#include "shaders.h"

#include <glad/gl.h>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

constexpr std::string_view vert_shader = R"(
    #version 450 core

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord_in;

    uniform mat4 model;
    uniform mat4 view_proj;

    out vec2 texcoord;

    void main() {
        gl_Position = view_proj * model * vec4(position, 1.0);
        texcoord = texcoord_in;
    }
)";

constexpr std::string_view frag_shader = R"(
	#version 450 core

	in vec2 texcoord;
	out vec4 fragcolor;

	layout(location = 0) uniform sampler2D albedo_texture;
	layout(binding = 0, std140) uniform Material {
		vec4 base_color;
		float metallic;
		float roughness;
	} material;

	void main() {
		// vec4 color = material.base_color;
		vec4 color = texture(albedo_texture, texcoord);
		fragcolor = color;
	}
)";

std::optional<GLuint> compile_program()
{
	GLint success;
	std::string log;
	auto program = glCreateProgram();

	std::vector<std::pair<std::string_view, GLuint>> shaders = {
		{vert_shader, glCreateShader(GL_VERTEX_SHADER)},
		{frag_shader, glCreateShader(GL_FRAGMENT_SHADER)}
	};

	for (auto [src, shader] : shaders) {
		const auto* data = src.data();
		auto size = static_cast<GLint>(src.size());
		glShaderSource(shader, 1, &data, &size);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			GLint length;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

			glGetShaderInfoLog(shader, length, nullptr, log.data());
			std::cout << "Failed to compile shader: " << log << "\n";
			return std::nullopt;
		}

		glAttachShader(program, shader);
	}

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		GLint length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &success);

		glGetProgramInfoLog(program, length, nullptr, log.data());
		std::cout << "Failed to link shaders:" << log << "\n";
		return std::nullopt;
	}

	return std::optional<GLuint>{program};
}
