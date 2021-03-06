#pragma once

#include <vector>
#include <memory>
#include <algorithm>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/MovableComponent.hpp"
#include "../include/Position.hpp"
#include "../include/CollidedComponent.hpp"
#include "../include/Physicable.hpp"
#include "../include/Carryable.hpp"
#include "../include/MagicSystem.hpp"

#include "../include/Camera.hpp"

class Collided {
	public:
		void run(std::vector<EntityPtr>& entitys) {
			std::vector<EntityPtr> erase, damaged;
			for(auto& entity : entitys) {
				if(entity->getComponent<Movable>() != nullptr) {
					CollidedPtr collided = entity->getComponent<CollidedComponent>();
					
					if(collided != nullptr) {
						for(auto& opponent : collided->m_entitys) {
							CarriedPtr carried = opponent->getComponent<Carried>();
							if(carried != nullptr) {
								// If the carried entity is in collision with its owner then we do not manage the collision
								if(carried->m_entity == entity) {
									continue;
								}
							}

							CollisablePtr<Cobble> box_opponent = opponent->getComponent<Collisable<Cobble>>();
							CollisablePtr<Cobble> box_entity = entity->getComponent<Collisable<Cobble>>();

							if(box_opponent != nullptr && box_entity != nullptr) {
								handleBoxBoxCollision(erase, damaged, entity, opponent);
							}

							//collided->m_entitys.erase(opponent);
							CollidedPtr collided_opponent = opponent->getComponent<CollidedComponent>();
							std::vector<EntityPtr>::iterator it = std::find(collided_opponent->m_entitys.begin(), collided_opponent->m_entitys.end(), entity);
							collided_opponent->m_entitys.erase(it);

							// When we have process the result of a collision
							if(collided_opponent->m_entitys.empty()) {
								opponent->deleteComponent<CollidedComponent>();
							}
						}
						entity->deleteComponent<CollidedComponent>();
					}
				}
	        }
			for(auto entity : damaged) {
	        	KillablePtr life = entity->getComponent<Killable>();
	        	if(life->m_life <= 0) {
	        		std::cout << life->m_life << std::endl;
	        		erase.push_back(entity);
	        	}
	        }

	        for(auto entity : erase) {
	        	std::vector<EntityPtr>::iterator it = std::find(entitys.begin(), entitys.end(), entity);
	        	if(it != entitys.end()) {
	        		entitys.erase(it);
	        	}
	        }
		}

		void handleBoxBoxCollision(std::vector<EntityPtr>& erase,
								   std::vector<EntityPtr>& damaged,
								   const EntityPtr entity,
								   const EntityPtr opponent) {
			TriggerablePtr throwable_entity = entity->getComponent<Triggerable>();
			TriggerablePtr throwable_opponent = opponent->getComponent<Triggerable>();
			if(throwable_entity != nullptr && throwable_opponent != nullptr) {
				return;
			}

			if(throwable_entity != nullptr && std::find(damaged.begin(), damaged.end(), opponent) == damaged.end()) {
				if(opponent != throwable_entity->m_thrower) {
					KillablePtr opponent_life = opponent->getComponent<Killable>();

					if(opponent_life != nullptr) {
						//std::cout << "damage !" << std::endl;
						EffectCollidedPtr effect = entity->getComponent<EffectCollided>();
						effectCollided(effect, opponent);
						damaged.push_back(opponent);
					}
					erase.push_back(entity);
				}
			} else if(throwable_opponent != nullptr && std::find(damaged.begin(), damaged.end(), entity) == damaged.end()) {
				if(entity != throwable_opponent->m_thrower) {
					KillablePtr entity_life = entity->getComponent<Killable>();

					if(entity_life != nullptr) {
						//std::cout << "damage !" << std::endl;
						EffectCollidedPtr effect = opponent->getComponent<EffectCollided>();
						effectCollided(effect, entity);
						damaged.push_back(entity);
					}
					erase.push_back(opponent);
				}
			} else {
				CollisablePtr<Cobble> box_opponent = opponent->getComponent<Collisable<Cobble>>();
				CollisablePtr<Cobble> box_entity = entity->getComponent<Collisable<Cobble>>();

				glm::vec3 overlap = getOverlapVector(box_opponent, box_entity);
				//glm::vec3 delta = glm::vec3(overlap.x, 0.f, overlap.z);
				glm::vec3 delta = glm::vec3(overlap.x, 0.f, 0.f);
				if(overlap.z < overlap.x) {
					delta = glm::vec3(0.f, 0.f, overlap.z);
				}
				if(overlap.y < overlap.x && overlap.y < overlap.z) {
					delta = glm::vec3(0.f, overlap.y, 0.f);

					PhysicPtr physic = entity->getComponent<Physic>();
					if(physic != nullptr) {
						physic->m_jump = false;
					}
				}
				
				if(box_entity->m_position.x <= box_opponent->m_position.x) {
					delta.x *= -1.f; 
				} 
				if(box_entity->m_position.y <= box_opponent->m_position.y) {
					delta.y *= -1.f; 
				} 
				if(box_entity->m_position.z <= box_opponent->m_position.z) {
					delta.z *= -1.f; 
				}

				MovablePtr movable = entity->getComponent<Movable>();

				movable->m_position += delta;
				box_entity->m_position += delta;

				PhysicPtr physic = entity->getComponent<Physic>();
				if(physic != nullptr && physic->m_gravity > 0.f && delta.y > 0.f) {
					physic->m_gravity = 0.f;
				}

				box_entity->m_box->translateLocalMatrix(delta);
				movable->m_heritance = glm::translate(movable->m_heritance, delta*glm::vec3(SIZE_TILE));

				RenderableComponentPtr renderable = entity->getComponent<RenderableComponent>();
				if(renderable != nullptr) {
					renderable->m_renderable->setHeritanceMatrix(movable->m_heritance * glm::toMat4(movable->m_quat));
				}
			}

			MotionPtr<LineUpDown> motion = entity->getComponent<Motion<LineUpDown>>();
			if(motion != nullptr) {
				motion->m_first_to_second = not motion->m_first_to_second;
			}					
		}

		void effectCollided(const EffectCollidedPtr effects, EntityPtr opponent) {
			for(auto& effect : effects->m_effects) {
				effect->m_script(effect, opponent);
			} 
		}

		const glm::vec3 getOverlapVector(const CollisablePtr<Cobble> cobble1, const CollisablePtr<Cobble> cobble2) {
			glm::vec3 overlap;
			float x_min = cobble1->m_position.x;
			if(cobble2->m_position.x < cobble1->m_position.x) {
				x_min = cobble2->m_position.x;
				overlap.x = x_min + cobble2->m_size.x - cobble1->m_position.x;
			} else {
				overlap.x = x_min + cobble1->m_size.x - cobble2->m_position.x;
			}

			float y_min = cobble1->m_position.y;
			if(cobble2->m_position.y < cobble1->m_position.y) {
				y_min = cobble2->m_position.y;
				overlap.y = y_min + cobble2->m_size.y - cobble1->m_position.y;
			} else {
				overlap.y = y_min + cobble1->m_size.y - cobble2->m_position.y;
			}

			float z_min = cobble1->m_position.z;
			if(cobble2->m_position.z < cobble1->m_position.z) {
				z_min = cobble2->m_position.z;
				overlap.z = z_min + cobble2->m_size.z - cobble1->m_position.z;
			} else {
				overlap.z = z_min + cobble1->m_size.z - cobble2->m_position.z;
			}

			return overlap;
		}

		void setPlayer(const EntityPtr player) {
			m_player = player;
		}

	private:
		EntityPtr		m_player;
};