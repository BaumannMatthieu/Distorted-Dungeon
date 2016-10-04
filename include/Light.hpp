#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Component.h"

template<typename Type>
struct Light : public Component, public Type {
	glm::vec3 m_ambiant;
	glm::vec3 m_diffuse;
	glm::vec3 m_specular;

	float m_constant;
	float m_linear;
	float m_quadratic;
};

template<typename Type>
using LightPtr = std::shared_ptr<Light<Type>>;

struct Ponctual {
	glm::vec3	m_position;
};



