#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class MeshOBJ {
	public:
		MeshOBJ(const std::string& filename);
		~MeshOBJ();

		const std::vector<glm::vec3>& getPositions() const;
		const std::vector<glm::vec3>& getNormals() const;
		const std::vector<glm::vec2>& getTexcoords() const;

	private:
		std::vector<glm::vec3> m_positions;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec2> m_texcoords;
};

using MeshOBJPtr = std::shared_ptr<MeshOBJ>;