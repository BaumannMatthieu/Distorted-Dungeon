#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"

struct Position : public Component {
	glm::vec2	m_position;
};

using PositionPtr = std::shared_ptr<Position>;