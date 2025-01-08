#pragma once

#include "gltf.h"

#include <fastgltf/types.hpp>

#include <vector>

class Camera {
public:
	fastgltf::math::fmat4x4 view_matrix() const;
	fastgltf::math::fmat4x4 rotation_matrix() const;
	void update_position(fastgltf::math::fvec3 new_pos);
	void update_velocity(fastgltf::math::fvec3 new_vel);
	void update_rotation(float yaw_add, float pitch_add);
private:
	fastgltf::math::fvec3 velocity;
	fastgltf::math::fvec3 position;

	float pitch { 0.0f };
	float yaw { 0.0f };
};

struct Node {
	LoadedGLTF& gltf;
	fastgltf::math::fmat4x4 transform;
};

struct Scene {
	std::vector<Node> nodes;
};
