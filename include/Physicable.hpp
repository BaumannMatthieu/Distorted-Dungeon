#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"


struct Physic : public Component {
	float m_gravity;
	float m_mass;

	bool m_jump;
};

using PhysicPtr = std::shared_ptr<Physic>;

