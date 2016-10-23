#pragma once

#include <vector>
#include <memory>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/MovableComponent.hpp"


template<typename Type>
struct Motion : public Component, public Type {
};

struct LineUpDown {
	glm::vec3	m_first;
	glm::vec3	m_second;
	bool 		m_first_to_second;
};

struct Instant {
	glm::vec3	m_first;
	glm::vec3	m_second;
};

// Rotation between alpha and beta
struct Rotation {
	glm::quat		m_alpha;
	glm::quat		m_beta;

	unsigned int    m_time_start;
	unsigned int    m_duration;

	glm::mat4		m_heritance;
};

template<typename Type>
using MotionPtr = std::shared_ptr<Motion<Type>>;

class MotionManager {
	public:
		void run(std::vector<EntityPtr>& entitys) {
			for(auto& entity : entitys) {
				MotionPtr<LineUpDown> motion = entity->getComponent<Motion<LineUpDown>>();
				MovablePtr movable = entity->getComponent<Movable>();
				if(motion != nullptr && movable != nullptr) {
					if(motion->m_first_to_second) {
						glm::vec3 first_to_position = movable->m_position - motion->m_first;
						glm::vec3 first_to_second = motion->m_second - motion->m_first;
						if(glm::length(first_to_position) >= glm::length(first_to_second)) {
							motion->m_first_to_second = false;
							continue;
						}

						movable->m_direction = glm::normalize(first_to_second);
					} else {
						glm::vec3 second_to_position = movable->m_position - motion->m_second;
						glm::vec3 second_to_first = motion->m_first - motion->m_second;
						if(glm::length(second_to_position) >= glm::length(second_to_first)) {
							motion->m_first_to_second = true;
							continue;
						}

						movable->m_direction = glm::normalize(second_to_first);
					}
				}

				MotionPtr<Instant> motion_instant = entity->getComponent<Motion<Instant>>();
				if(motion_instant != nullptr && movable != nullptr) {
					movable->m_position = motion_instant->m_second;
					movable->m_speed = 0.f;
				}

				MotionPtr<Rotation> motion_rotate = entity->getComponent<Motion<Rotation>>();
				if(motion_rotate != nullptr) {
					float t = static_cast<float>((SDL_GetTicks() - motion_rotate->m_time_start))/(motion_rotate->m_duration);
					if(t > 1.0f) {
						entity->deleteComponent<Motion<Rotation>>();
						t = 1.0f;
					}

					glm::quat quat;
					if(SDL_GetTicks() < motion_rotate->m_time_start + motion_rotate->m_duration/2.f) {
						quat = glm::slerp(motion_rotate->m_alpha, motion_rotate->m_beta, 2.f*t);
					} else {
						quat = glm::slerp(motion_rotate->m_alpha, motion_rotate->m_beta, 2.f - 2.f*t);
					}

					RenderableComponentPtr render = entity->getComponent<RenderableComponent>();
					if(render != nullptr) {
						render->m_renderable->setHeritanceMatrix(motion_rotate->m_heritance*glm::toMat4(quat));
					}

					/*CollisablePtr<Cobble> collisable = entity->getComponent<Collisable<Cobble>>();
					if(collisable != nullptr) {
						//DrawablePtr parent = render->m_renderable->getParent();
						//RenderableComponentPtr render_player = m_player->getComponent<RenderableComponent>();

						entity->deleteComponent<Collisable<Cobble>>();
						//System<Collider>::attachBoundingBoxCollision(entity, m_player);

						//CollisablePtr<Cobble> col = entity->getComponent<Collisable<Cobble>>();
						//col->m_box->setParent(render_player->m_renderable);
						//collisable->m_box->setLocalMatrix(motion_rotate->m_local*glm::toMat4(quat));
					}*/
				}
	        }
		}

		void setPlayer(const EntityPtr player) {
			m_player = player;
		}
	private:
		EntityPtr m_player;
};