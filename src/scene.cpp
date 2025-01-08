#include "scene.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

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

void Camera::update_position(glm::vec3 new_pos)
{
	position = new_pos;
}


void Camera::update_velocity(glm::vec3 new_vel)
{
	velocity = new_vel;
}

void Camera::update_rotation(float yaw_add, float pitch_add)
{
	yaw += yaw_add;
	pitch += pitch_add;
}
