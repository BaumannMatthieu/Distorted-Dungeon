#pragma once

#include <sstream>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/RenderableComponent.hpp"
#include "../include/Light.hpp"

#include "../include/Camera.hpp"

class Render {
	public:
	void run(std::vector<EntityPtr>& entitys) {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		std::vector<EntityPtr> point_lights_entitys = EntityManager::getEntitysByComponent<Light<Ponctual>>(entitys);
		std::vector<EntityPtr> renderable_entitys = EntityManager::getEntitysByComponent<RenderableComponent>(entitys);

		//std::cout << "1 : " << renderable_entitys.size() << std::endl;
		deleteOutsideFrustumView(renderable_entitys);
		//std::cout << "2 : " << renderable_entitys.size() << std::endl;

		for(auto& entity : renderable_entitys) {
			RenderableComponentPtr renderable = entity->getComponent<RenderableComponent>();
            
			ShaderProgramPtr shader = renderable->m_renderable->getShaderProgram();
			shader->bind();

			int viewLocation = shader->getUniformLocation("view");
			int projectionLocation = shader->getUniformLocation("projection");

			int numLightsLocation = shader->getUniformLocation("num_lights");
			glUniform1i(numLightsLocation, point_lights_entitys.size());

			for(unsigned int i = 0; i < point_lights_entitys.size(); ++i) {
				LightPtr<Ponctual> light = point_lights_entitys[i]->getComponent<Light<Ponctual>>();
				std::stringstream ss;
				ss << i;
				std::string light_str = "light[" + ss.str() + "].";

				int lightPositionLocation = shader->getUniformLocation(light_str + "position");
				glUniform3fv(lightPositionLocation, 1, glm::value_ptr(light->m_position*glm::vec3(SIZE_TILE)));

				int lightAmbiantColorLocation = shader->getUniformLocation(light_str + "ambiant");
				glUniform3fv(lightAmbiantColorLocation, 1, glm::value_ptr(light->m_ambiant));
				int lightDiffuseColorLocation = shader->getUniformLocation(light_str + "diffuse");
				glUniform3fv(lightDiffuseColorLocation, 1, glm::value_ptr(light->m_diffuse));
				int lightSpecularColorLocation = shader->getUniformLocation(light_str + "specular");
				glUniform3fv(lightSpecularColorLocation, 1, glm::value_ptr(light->m_specular));

				int lightConstantLocation = shader->getUniformLocation(light_str + "constant");
				glUniform1f(lightConstantLocation, light->m_constant);
				int lightLinearLocation = shader->getUniformLocation(light_str + "linear");
				glUniform1f(lightLinearLocation, light->m_linear);
				int lightQuadraticLocation = shader->getUniformLocation(light_str + "quadratic");
				glUniform1f(lightQuadraticLocation, light->m_quadratic);
			}
		
			glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->getMatrixView()));
			glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->getMatrixProjection()));

			renderable->m_renderable->draw();

			shader->unbind();
        }
	}

	void deleteOutsideFrustumView(std::vector<EntityPtr>& entitys) {
		const Camera::FrustumView& frustum = camera->getFrustumView();
		std::vector<EntityPtr> inside_frustum_entitys;
		for(auto& entity : entitys) {
			if(!outside(entity, frustum)) {
				inside_frustum_entitys.push_back(entity);
			}	
		}
		entitys = inside_frustum_entitys;
	}

	bool outside(const EntityPtr entity, const Camera::FrustumView& frustum) {
		CollisablePtr<Cobble> collisable = entity->getComponent<Collisable<Cobble>>();
		if(collisable != nullptr) {
			glm::vec3 p = collisable->m_position*glm::vec3(SIZE_TILE);
			glm::vec3 size = collisable->m_size*glm::vec3(SIZE_TILE);

			std::array<glm::vec3, 8> points;
			points[0] = p;
			points[1] = p + glm::vec3(size.x, 0.f, 0.f);
			points[2] = p + glm::vec3(0.f, size.y, 0.f);
			points[3] = p + glm::vec3(0.f, 0.f, size.z);
			points[4] = p + glm::vec3(size.x, size.y, 0.f);
			points[5] = p + glm::vec3(size.x, 0.f, size.z);
			points[6] = p + glm::vec3(0.f, size.y, size.z);
			points[7] = p + size;

			for(unsigned int i = 0; i < 8; ++i) {
				bool point_inside = true;
				for(unsigned int j = 0; j < 6; ++j) {
					if(glm::dot(frustum.m_planes[j].n, points[i]) + frustum.m_planes[j].d >= 0.f) {
						point_inside = false;
						break;
					}
				}
				if(point_inside) {
					return false;
				}
			}
		}
		return true;
	}

	bool point_inside(const glm::vec3& p, const Camera::Plane& plane) {
		return (glm::dot(plane.n, p) + plane.d >= 0.f);
	}
};