#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"

template<typename Type>
struct Collisable : public Component, public Type {
};

struct Square {
	glm::vec2	m_position;
	glm::vec2	m_position_start;
	glm::vec2	m_size;
};

struct Cobble {
	glm::vec3			m_position;
	glm::vec3			m_size;
	RenderablePtr<Box>	m_box;
};

struct BoundingBox {
	glm::vec3			m_position;
	glm::vec3			m_size;
	RenderablePtr<Box>	m_box;
};

struct Circle {
	glm::vec2	m_center;
	float		m_radius;
};

struct Point {
	glm::vec2	m_point;
};

template<typename Type>
using CollisablePtr = std::shared_ptr<Collisable<Type>>;

