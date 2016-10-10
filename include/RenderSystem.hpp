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

		for(auto& entity : entitys) {
			RenderableComponentPtr renderable = entity->getComponent<RenderableComponent>();
            
            if(renderable != nullptr) {
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

			/*CollisablePtr<Cobble> AABB = entity->getComponent<Collisable<Cobble>>();
            if(AABB != nullptr) {
            	std::cout << "dfsfsf" << std::endl;
            	ShaderProgramPtr shader = AABB->m_box->getShaderProgram();
				shader->bind();
				AABB->m_box->draw();
				shader->unbind();
            }*/
        }
	}
};