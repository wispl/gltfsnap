#include "shaders.h"

#include <glad/gl.h>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

constexpr std::string_view vert_shader = R"(
    #version 450 core

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec2 texcoord;

    uniform mat4 model;
    uniform mat4 view_proj;

    out vec2 out_texcoord;

    void main() {
        gl_Position = view_proj * model * vec4(position, 1.0);
        out_texcoord = texcoord;
    }
)";

constexpr std::string_view frag_shader = R"(
	#version 450 core

	in vec2 texcoord;
	out vec4 fragcolor;

	layout(location = 0) uniform sampler2D albedoTexture;
	uniform vec4 base_color;

	void main() {
		vec4 color = base_color;
		// color *= texture(albedo_texture, texcoord);
		fragcolor = base_color;
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
