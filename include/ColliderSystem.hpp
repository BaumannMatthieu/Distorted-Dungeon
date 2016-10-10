#pragma once

#include <vector>
#include <memory>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/MovableComponent.hpp"
#include "../include/Position.hpp"
#include "../include/CollisableComponent.hpp"
#include "../include/CollidedComponent.hpp"

#include "../include/Dunjeon.hpp"
#include "../include/Camera.hpp"

class Collider {
	public:
		void run(std::vector<EntityPtr>& entitys) {
			for(auto& entity : entitys) {
				if(entity->getComponent<CollidedComponent>() != nullptr) {
					continue;
				}

				MovablePtr movable = entity->getComponent<Movable>();
				//PositionPtr position = entity->getComponent<Position>();
				if(movable != nullptr) {
					for(auto& entity_collision : entitys) {
						bool collision = false;
						if(entity_collision != entity) {
							// Collisable Square
							CollisablePtr<Square> square = entity_collision->getComponent<Collisable<Square>>();
							if(square != nullptr) {
								//glm::vec2 start = glm::vec2(movable->m_position.x, movable->m_position.z);
								glm::vec3 ray = movable->m_position;

								if(movable->m_speed < 0.f) {
									ray -= movable->m_direction/5.f;
								} else {
									ray += movable->m_direction/5.f;
								}

								glm::vec2 p = glm::vec2(ray.x, ray.z);

								//glm::vec2 pos_square = square->m_position;
								//glm::vec2 size_square = square->m_size;

								/*if(intersect(start, end, pos_square, pos_square + glm::vec2(size_square.x, 0.f)) ||
									intersect(start, end, pos_square + glm::vec2(size_square.x, 0.f), pos_square + size_square) ||
									intersect(start, end, pos_square + size_square, pos_square + glm::vec2(0.f, size_square.y)) ||
									intersect(start, end, pos_square + glm::vec2(0.f, size_square.y), pos_square)) {
									collision = true;
								}*/

								if(point_in_square(p, square)) {
									collision = true;	
								}
							}

							CollisablePtr<Cobble> cobble = entity->getComponent<Collisable<Cobble>>();
							if(cobble != nullptr) {
								// Computation of the Bounding box
								RenderableComponentPtr render = entity->getComponent<RenderableComponent>();
								if(render != nullptr) {
									std::vector<glm::vec3> positions = render->m_renderable->getPositions();

									//glm::vec3 size = getSizePositions(positions);
									//glm::vec3 barycenter = getBarycenterPositions(positions);
									
									//cobble->m_position = (barycenter - size/2.f)/unitWorld;
									//cobble->m_size = size/unitWorld;

									/*cobble->m_box->setDimensions(barycenter, size);*/
								}
								// Collisable Cobble
								CollisablePtr<Cobble> cobble_opponent = entity_collision->getComponent<Collisable<Cobble>>();
								if(cobble_opponent != nullptr) {			
									if(AABB_AABB(cobble_opponent, cobble)) {
										collision = true;	
									}
								}
							}

							if(collision) {
								CollidedPtr collided = entity->getComponent<CollidedComponent>();
								if(collided == nullptr) {
									collided = std::make_shared<CollidedComponent>();
									collided->m_entitys.push_back(entity_collision);
									entity->addComponent<CollidedComponent>(collided);
								} else {
									collided->m_entitys.push_back(entity_collision);
								}

								CollidedPtr collided_opponent = entity_collision->getComponent<CollidedComponent>();
								if(collided_opponent == nullptr) {
									collided_opponent = std::make_shared<CollidedComponent>();
									collided_opponent->m_entitys.push_back(entity);
									entity_collision->addComponent<CollidedComponent>(collided_opponent);
								} else {
									collided_opponent->m_entitys.push_back(entity);
								}
								//break;
							}
						}
					} 
				}
	        }
		}

		static bool point_in_square(const glm::vec2& point, const CollisablePtr<Square> square) {
			return (point.x >= square->m_position.x &&
					point.x <= square->m_position.x + square->m_size.x &&
					point.y >= square->m_position.y &&
					point.y <= square->m_position.y + square->m_size.y);
		}

		static bool point_in_cobble(const glm::vec3& point, const CollisablePtr<Cobble> cobble) {
			return (point.x >= cobble->m_position.x &&
					point.x <= cobble->m_position.x + cobble->m_size.x &&
					point.y >= cobble->m_position.y &&
					point.y <= cobble->m_position.y + cobble->m_size.y &&
					point.z >= cobble->m_position.z &&
					point.z <= cobble->m_position.z + cobble->m_size.z);
		}

		static bool AABB_AABB(const CollisablePtr<Cobble> cobble1, const CollisablePtr<Cobble> cobble2) {
			bool no_overlap = ((cobble1->m_position.x + cobble1->m_size.x <= cobble2->m_position.x) ||
							   (cobble2->m_position.x + cobble2->m_size.x <= cobble1->m_position.x) ||
							   (cobble1->m_position.y + cobble1->m_size.y <= cobble2->m_position.y) ||
							   (cobble2->m_position.y + cobble2->m_size.y <= cobble1->m_position.y) ||
							   (cobble1->m_position.z + cobble1->m_size.z <= cobble2->m_position.z) ||
							   (cobble2->m_position.z + cobble2->m_size.z <= cobble1->m_position.z));

			return !no_overlap;
		}

		static bool intersect(const glm::vec2& s1_p1, const glm::vec2& s1_p2, const glm::vec2& s2_p1, const glm::vec2& s2_p2) {
			glm::vec2 r = s1_p2 - s1_p1;
			glm::vec2 s = s2_p2 - s2_p1;

			glm::vec2 p = s1_p1;
			glm::vec2 q = s2_p1;

			float cross = r.x*s.y - r.y*s.x;

			if(cross == 0)
				return false;

			float t = ((q.x - p.x)*s.y - (q.y - p.y)*s.x)/cross;
			float u = ((q.x - p.x)*r.y - (q.y - p.y)*r.x)/cross;

			return t >= 0.f && t <= 1.f && u >= 0.f && u <= 1.f; 
		}

		static glm::vec3 getSizePositions(const DrawablePtr& drawable, float scale) {
			std::vector<glm::vec3> positions = drawable->getPositions();
			glm::vec3 current_vertex = positions[0];

			float x_min = current_vertex.x;
			float x_max = current_vertex.x;
			float y_min = current_vertex.y;
			float y_max = current_vertex.y;
			float z_min = current_vertex.z;
			float z_max = current_vertex.z;

			for(unsigned int i = 1; i < positions.size(); ++i) {
				current_vertex = positions[i];

				if(current_vertex.x < x_min) {
					x_min = current_vertex.x;
				} else if(current_vertex.x > x_max) {
					x_max = current_vertex.x;
				}

				if(current_vertex.y < y_min) {
					y_min = current_vertex.y;
				} else if(current_vertex.y > y_max) {
					y_max = current_vertex.y;
				}

				if(current_vertex.z < z_min) {
					z_min = current_vertex.z;
				} else if(current_vertex.z > z_max) {
					z_max = current_vertex.z;
				}
			}

			glm::vec3 size(x_max - x_min,
							 y_max - y_min,
							 z_max - z_min);
			return size*glm::vec3(scale);
		}

		static glm::vec3 getBarycenterPositions(const std::vector<glm::vec3>& positions) {
			float x_min = positions[0].x;
			float x_max = positions[0].x;
			float y_min = positions[0].y;
			float y_max = positions[0].y;
			float z_min = positions[0].z;
			float z_max = positions[0].z;

			for(unsigned int i = 1; i < positions.size(); ++i) {
				if(positions[i].x < x_min) {
					x_min = positions[i].x;
				} else if(positions[i].x > x_max) {
					x_max = positions[i].x;
				}

				if(positions[i].y < y_min) {
					y_min = positions[i].y;
				} else if(positions[i].y > y_max) {
					y_max = positions[i].y;
				}

				if(positions[i].z < z_min) {
					z_min = positions[i].z;
				} else if(positions[i].z > z_max) {
					z_max = positions[i].z;
				}
			}

			glm::vec3 center((x_max + x_min)*0.5f,
							 (y_max + y_min)*0.5f,
							 (z_max + z_min)*0.5f);
			return center;
		}

		/*static void attachBoundingBoxCollision(EntityPtr entity, const EntityPtr parent) {
			RenderableComponentPtr renderable = entity->getComponent<RenderableComponent>();
			if(renderable != nullptr) {
				CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
				MovablePtr movable = entity->getComponent<Movable>();

				std::vector<glm::vec3> positions = renderable->m_renderable->getPositions();
				collisable->m_position = getBarycenterPositions(positions) + movable->m_position;
				collisable->m_size = getSizePositions(positions);

				collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																		  (collisable->m_position - collisable->m_size/2.f),
																		  (collisable->m_size),
																		  glm::vec4(1.f, 1.f, 1.f, 1.f));
				collisable->m_box->setLocalMatrix(renderable->m_renderable->getLocalMatrix() * collisable->m_box->getLocalMatrix());
				collisable->m_box->setHeritanceMatrix(renderable->m_renderable->getHeritanceMatrix() * collisable->m_box->getHeritanceMatrix());

				if(parent != nullptr) {
					RenderableComponentPtr renderable_parent = parent->getComponent<RenderableComponent>();
					
					collisable->m_box->setParent(renderable_parent->m_renderable);
					renderable->m_renderable->setParent(renderable_parent->m_renderable);
				} 

				entity->addComponent<Collisable<Cobble>>(collisable);
			}
		}*/

		void setDungeon(DungeonPtr dungeon) {
			m_dungeon = dungeon;
		}
	private:
	
		DungeonPtr		m_dungeon;
};