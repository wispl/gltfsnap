#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <vector>

// Forward declare
class LoadedGLTF;

class Camera {
public:
	glm::mat4 view_matrix() const;
	glm::mat4 rotation_matrix() const;
	void update_position(glm::vec3 new_pos);
	void update_velocity(glm::vec3 new_vel);
	void update_rotation(float yaw_add, float pitch_add);
private:
	glm::vec3 velocity;
	glm::vec3 position;

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
