#include "scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

// Most of the code is from https://vkguide.dev/docs/new_chapter_5/interactive_camera/

uint32_t Node::num_entities = 0;

glm::mat4 Camera::view_matrix() const
{
	auto translation = glm::translate(glm::mat4(1.0f), position);
	auto rotation = rotation_matrix();
	return glm::inverse(translation * rotation);
}

glm::mat4 Camera::rotation_matrix() const
{
	auto pitch_quat = glm::angleAxis(pitch, glm::vec3{ 1.0f, 0.0f, 0.0f });
	auto yaw_quat = glm::angleAxis(yaw, glm::vec3{ 0.0f, -1.0f, 0.0f });
	return glm::toMat4(yaw_quat) * glm::toMat4(pitch_quat);
}

void Camera::update()
{
	position += glm::vec3(rotation_matrix() * glm::vec4(velocity * 0.5f, 0.0f));
}

void Camera::move(Direction direction, float delta_time)
{
	switch (direction) {
	case Direction::FORWARD:  velocity.z = -speed * delta_time; break;
	case Direction::BACKWARD: velocity.z =  speed * delta_time; break;
	case Direction::RIGHT:	  velocity.x =  speed * delta_time; break;
	case Direction::LEFT:	  velocity.x = -speed * delta_time; break;
	};
}

void Camera::stop(Direction direction)
{
	switch (direction) {
	case Direction::FORWARD:  velocity.z = 0; break;
	case Direction::BACKWARD: velocity.z = 0; break;
	case Direction::RIGHT:	  velocity.x = 0; break;
	case Direction::LEFT:	  velocity.x = 0; break;
	};
}

void Camera::rotate(float yaw_add, float pitch_add)
{
	yaw += yaw_add;
	pitch += pitch_add;
}

void Camera::set_speed(float new_speed)
{
	speed = new_speed;
}

void Camera::set_position(glm::vec3 new_pos)
{
	position = new_pos;
}
