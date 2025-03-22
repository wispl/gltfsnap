#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <vector>

// Forward declare
class LoadedGLTF;

enum class Direction { FORWARD, RIGHT, LEFT, BACKWARD };

class Camera {
public:
	glm::mat4 view_matrix() const;
	glm::mat4 rotation_matrix() const;

	void update();
	void move(Direction direction, float delta_time);
	// TODO: remove `stop` after camera is exposed outside of `Renderer`
	void stop(Direction direction);
	void rotate(float yaw_add, float pitch_add);

	void set_position(glm::vec3 new_pos);
	void set_speed(float new_vel);
private:
	glm::vec3 position, velocity { 0.0f };
	float speed = 1.0f;

	float pitch { 0.0f };
	float yaw { 0.0f };
};

struct Node {
public:
	uint32_t id;
	std::shared_ptr<LoadedGLTF> gltf;
	glm::mat4 transform;

	Node(std::shared_ptr<LoadedGLTF> gltf) : gltf(gltf), id(num_entities++) {};
	Node(std::shared_ptr<LoadedGLTF> gltf, glm::mat4 transform) : gltf(gltf), transform(transform), id(num_entities++) {};
	bool operator==(const Node& other) {
		return (id == other.id);
	}
private:
	// TODO: we might need a better way to generate uuids
	static uint32_t num_entities;
};

struct Scene {
	// TODO: buffer changes?
	std::vector<Node> nodes;
};
