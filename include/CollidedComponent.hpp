#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"
#include "../include/Entity.h"

struct CollidedComponent : public Component {
	std::vector<EntityPtr>			m_entitys;
};

using CollidedPtr = std::shared_ptr<CollidedComponent>;

struct Blocked : public Component {
};