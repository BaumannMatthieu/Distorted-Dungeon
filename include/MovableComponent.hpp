#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../include/Component.h"

struct Movable : public Component {
	glm::vec3	m_direction;
	glm::vec3	m_position;
	float		m_speed;

	glm::quat	m_quat;
	glm::mat4	m_heritance;
};

using MovablePtr = std::shared_ptr<Movable>;