#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"
#include "../include/renderable.hpp"

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

static glm::vec3 getSizePositions(const DrawablePtr& drawable, float scale) {
	std::vector<glm::vec3> positions = drawable->getPositions();
	glm::vec3 current_vertex = positions[0];

	float x_min = current_vertex.x;
	float x_max = current_vertex.x;
	float y_min = current_vertex.y;
	float y_max = current_vertex.y;
	float z_min = current_vertex.z;
	float z_max = current_vertex.z;

	for(unsigned int i = 1; i < positions.size(); ++i) {
		current_vertex = positions[i];

		if(current_vertex.x < x_min) {
			x_min = current_vertex.x;
		} else if(current_vertex.x > x_max) {
			x_max = current_vertex.x;
		}

		if(current_vertex.y < y_min) {
			y_min = current_vertex.y;
		} else if(current_vertex.y > y_max) {
			y_max = current_vertex.y;
		}

		if(current_vertex.z < z_min) {
			z_min = current_vertex.z;
		} else if(current_vertex.z > z_max) {
			z_max = current_vertex.z;
		}
	}

	glm::vec3 size(x_max - x_min,
					 y_max - y_min,
					 z_max - z_min);
	return size*glm::vec3(scale);
}

