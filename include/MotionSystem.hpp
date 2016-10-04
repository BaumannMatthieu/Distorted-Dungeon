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
	float 		m_alpha;
	float 		m_beta;

	glm::vec3	m_axis;

	bool		m_alpha_to_beta;
	float		m_angle;
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
					if(motion_rotate->m_alpha_to_beta) {
						
					} else {

					}
				}
	        }
		}
};