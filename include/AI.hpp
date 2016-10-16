#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/Component.h"

#include "../include/MovableComponent.hpp"
#include "../include/CollisableComponent.hpp"
#include "../include/MotionSystem.hpp"

struct AI : public Component {
};

using AIPtr = std::shared_ptr<AI>;

class AISystem {
public:
	void run(std::vector<EntityPtr>& entitys) {
		MovablePtr movable = m_player->getComponent<Movable>();

		std::vector<EntityPtr> AIEntitys = EntityManager::getEntitysByComponent<AI>(entitys);
		for(auto& entity : AIEntitys) {
			MovablePtr movable_ai = entity->getComponent<Movable>();
			glm::vec3 distance = movable->m_position - movable_ai->m_position;
			float distance_norme = glm::length(distance);

			movable_ai->m_speed = 0.f;

			if(distance_norme < 5.f && distance_norme >= 0.5f) {
				movable_ai->m_direction = distance;
				glm::normalize(movable_ai->m_direction);
				movable_ai->m_speed = 1.5f;
			}
		}
	}

	void setPlayer(const EntityPtr player) {
		m_player = player;
	}
	
	private:
		EntityPtr		m_player;
};


