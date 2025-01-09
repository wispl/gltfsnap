#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <vector>

// Forward declare
class LoadedGLTF;

enum class Direction { FOWARD, RIGHT, LEFT, BACKWARD };

class Camera {
public:
	glm::mat4 view_matrix() const;
	glm::mat4 rotation_matrix() const;

	void update();
	void move(Direction direction, float delta_time);
	void rotate(float yaw_add, float pitch_add);

	void set_position(glm::vec3 new_pos);
	void set_speed(float new_vel);
private:
	glm::vec3 position, velocity;
	float speed = 1.0f;

	float pitch { 0.0f };
	float yaw { 0.0f };
};

struct Node {
	LoadedGLTF& gltf;
	glm::mat4 transform;
};

struct Scene {
	std::vector<Node> nodes;
};
