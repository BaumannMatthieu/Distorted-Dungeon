#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Plane {
	public:
		Plane();
		~Plane();

		static void initialize(std::vector<glm::vec3>& positions,
							   std::vector<glm::vec3>& normals,
							   std::vector<glm::vec4>& colors,
							   std::vector<glm::vec2>& texcoords);
};
