#include "scene.h"

#include <fastgltf/types.hpp>
#include <fastgltf/math.hpp>

#include <cmath>

static fastgltf::math::fquat angle_axis(float angle, fastgltf::math::fvec3 axis)
{
	float sin = std::sin(angle / 2);
	float x = axis[0] * sin;
	float y = axis[1] * sin;
	float z = axis[2] * sin;
	float w = std::cos(angle / 2);
	return fastgltf::math::fquat(x, y, z, w);
}

fastgltf::math::fmat4x4 Camera::view_matrix() const
{
	fastgltf::math::fmat4x4 identity;
	auto translation = fastgltf::math::translate(identity, position);
	auto rotation = rotation_matrix();
	return fastgltf::math::affineInverse(translation * rotation);
}

fastgltf::math::fmat4x4 Camera::rotation_matrix() const
{
	auto pitch_quat = angle_axis(pitch, fastgltf::math::fvec3{ 1.0f, 0.0f, 0.0f });
	auto yaw_quat = angle_axis(yaw, fastgltf::math::fvec3{ 0.0f, -1.0f, 0.0f });
	return fastgltf::math::fmat4x4(fastgltf::math::asMatrix(pitch_quat))
	     * fastgltf::math::fmat4x4(fastgltf::math::asMatrix(yaw_quat));
}

void Camera::update_position(fastgltf::math::fvec3 new_pos)
{
	position = new_pos;
}


void Camera::update_velocity(fastgltf::math::fvec3 new_vel)
{
	velocity = new_vel;
}

void Camera::update_rotation(float yaw_add, float pitch_add)
{
	yaw += yaw_add;
	pitch += pitch_add;
}
