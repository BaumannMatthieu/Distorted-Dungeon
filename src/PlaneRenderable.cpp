#include <vector>
#include <memory>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/PlaneRenderable.hpp"
#include "../include/ShaderProgram.hpp"

#include "../include/Position.hpp"

Plane::Plane() {
}

Plane::~Plane() {

}

void Plane::initialize(std::vector<glm::vec3>& positions,
                       std::vector<glm::vec3>& normals,
					   std::vector<glm::vec4>& colors,
                       std::vector<glm::vec2>& texcoords) {
    positions.push_back(glm::vec3(-0.5f, 0.0f, 0.5f));
    positions.push_back(glm::vec3(0.5f, 0.0f, 0.5f));
    positions.push_back(glm::vec3(0.5f, 0.0f, -0.5f));
    positions.push_back(glm::vec3(-0.5f, 0.0f, 0.5f));
    positions.push_back(glm::vec3(0.5f, 0.0f, -0.5f));
    positions.push_back(glm::vec3(-0.5f, 0.0f, -0.5f));

    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    colors.push_back(glm::vec4(0.583f,  0.771f,  0.014f, 1.0f));
    colors.push_back(glm::vec4(0.609f,  0.115f,  0.436f, 1.0f));
    colors.push_back(glm::vec4(0.327f,  0.483f,  0.844f, 1.0f));
    colors.push_back(glm::vec4(0.822f,  0.569f,  0.201f, 1.0f));
    colors.push_back(glm::vec4(0.435f,  0.602f,  0.223f, 1.0f));
    colors.push_back(glm::vec4(0.310f,  0.747f,  0.185f, 1.0f));

    texcoords.push_back(glm::vec2(0.f, 0.f));
    texcoords.push_back(glm::vec2(1.0f, 0.f));
    texcoords.push_back(glm::vec2(1.0f, 1.0f));
    texcoords.push_back(glm::vec2(0.f, 0.f));
    texcoords.push_back(glm::vec2(1.0f, 1.0f));
    texcoords.push_back(glm::vec2(0.f, 1.0f));
}
