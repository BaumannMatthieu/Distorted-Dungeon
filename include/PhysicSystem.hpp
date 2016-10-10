#pragma once

#include <vector>
#include <memory>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/MovableComponent.hpp"
#include "../include/Physicable.hpp"

class PhysicManager {
	public:
		PhysicManager() : m_time(SDL_GetTicks()),
						  m_duration_jump(150) {
		}

		void run(std::vector<EntityPtr>& entitys) {
			if(m_time == 0) {
				m_time = SDL_GetTicks();
			}
			float dt = (float)(SDL_GetTicks() - m_time)/1000.f;
			/*if(dt > 0.020f) {
				dt = 0.020f;
			}*/

			SDL_PumpEvents();
			const Uint8 *keystate = SDL_GetKeyboardState(NULL);
			if(keystate[SDL_SCANCODE_R]) {
				MovablePtr movable = m_player->getComponent<Movable>();
				PhysicPtr physic = m_player->getComponent<Physic>();

				if(!physic->m_jump||(SDL_GetTicks() - m_time_start_jump) < m_duration_jump) {
					if(!physic->m_jump) {
						m_time_start_jump = SDL_GetTicks();
					}
					
					physic->m_jump = true;
				}	
			}

			for(auto& entity : entitys) {
				PhysicPtr physic = entity->getComponent<Physic>();
				MovablePtr movable = entity->getComponent<Movable>();

				glm::vec3 dx(0.f);

				if(movable != nullptr) {
					if(physic != nullptr) {
						glm::vec3 gravity_force = gravity(physic);
						glm::vec3 jump_force(0.f);

						// Force which is just for player : jump force
						if((SDL_GetTicks() - m_time_start_jump) < m_duration_jump && entity == m_player) {
							jump_force = glm::vec3(movable->m_direction.x*movable->m_speed*100.f, 400.f, movable->m_direction.z*movable->m_speed*100.f);
						}

						glm::vec3 velocity = movable->m_direction*movable->m_speed + (jump_force + gravity_force)*dt;

						if(glm::length(velocity) == 0) {
							movable->m_direction = velocity;
							movable->m_speed = 0.f;
							continue;
						}

						movable->m_direction = glm::normalize(velocity);
						movable->m_speed = glm::length(velocity);

						dx = velocity*dt;
						movable->m_position += dx;
					} else {
						glm::vec3 velocity = movable->m_direction*movable->m_speed;
						dx = velocity*dt;
					}

					movable->m_heritance = glm::translate(movable->m_heritance, dx*glm::vec3(SIZE_TILE));
					RenderableComponentPtr renderable = entity->getComponent<RenderableComponent>();
					if(renderable != nullptr) {
						renderable->m_renderable->setHeritanceMatrix(movable->m_heritance * glm::toMat4(movable->m_quat));
					}

					LightPtr<Ponctual> light = entity->getComponent<Light<Ponctual>>();
					if(light != nullptr) {
						light->m_position += dx;
					}

					CollisablePtr<Cobble> collisable = entity->getComponent<Collisable<Cobble>>();
					if(collisable != nullptr) {
						collisable->m_position += dx;
						
						collisable->m_box->translateLocalMatrix(dx);
					}
				}
			}
			m_time = SDL_GetTicks();
		}

		inline const glm::vec3 gravity(const PhysicPtr physic) const{
			return glm::vec3(0.f, -physic->m_gravity*physic->m_mass, 0.f);
		}

		void setPlayer(const EntityPtr player) {
			m_player = player;
		}

	private:
		unsigned int		m_time;

		unsigned int		m_time_start_jump;
		unsigned int		m_duration_jump;
		bool				m_jump;

		EntityPtr			m_player;
};