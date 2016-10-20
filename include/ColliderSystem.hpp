#pragma once

#include <vector>
#include <memory>
#include <map>
#include <set>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/MovableComponent.hpp"
#include "../include/Position.hpp"
#include "../include/CollisableComponent.hpp"
#include "../include/CollidedComponent.hpp"

#include "../include/Dunjeon.hpp"
#include "../include/Camera.hpp"

class HashMapCollisable {
	public:
		class Partition {
			public:
				Partition(const glm::vec3& position, const glm::vec3& size) : m_position(position),
																			  m_size(size) {
				}

				static bool intersect(const Partition& p1, const Partition& p2) {
					return true;
				}

				bool operator==(const Partition& p) {
					return (p.m_position == m_position && p.m_size == m_size);
				}

			public:
				glm::vec3	m_position;
				glm::vec3	m_size;
		};

		struct Comp {
			inline bool operator()(const Partition& p1, const Partition& p2) {
				if(p1.m_position.x != p2.m_position.x) {
					return p2.m_position.x > p1.m_position.x;
				}

				if(p2.m_position.y != p1.m_position.y) {
					return p2.m_position.y > p1.m_position.y;
				}

				if(p2.m_position.z != p1.m_position.z) {
					return p2.m_position.z > p1.m_position.z;
				}
				return false;
			}
		};

		void insert(const EntityPtr entity) {
			CollisablePtr<Cobble> box = entity->getComponent<Collisable<Cobble>>();
			if(box != nullptr) {
				std::set<Partition, Comp> partitions = getPartitionIntersect(Partition(box->m_position, box->m_size));

				for(const auto& p : partitions) {
					m_map[p].insert(entity);
				}
			}
		}

		void erase(const EntityPtr entity) {
			CollisablePtr<Cobble> box = entity->getComponent<Collisable<Cobble>>();
			if(box != nullptr) {
				std::set<Partition, Comp> partitions = getPartitionIntersect(Partition(box->m_position, box->m_size));

				for(const auto& p : partitions) {
					m_map[p].erase(entity);
				}
			}
		}

		std::set<EntityPtr> getNearestEntitys(const EntityPtr entity) {
			std::set<EntityPtr> near;

			std::set<Partition, Comp> partitions;

			CollisablePtr<Cobble> box = entity->getComponent<Collisable<Cobble>>();
			if(box != nullptr) {
				partitions = getPartitionIntersect(Partition(box->m_position, box->m_size));
			}

			for(const auto& p : partitions) {
				std::set<EntityPtr>& near_p = m_map[p];
				for(auto& n : near_p) {
					near.insert(n);
				}
			}
			//std::cout << near.size() << std::endl;
			return near;
		}

	HashMapCollisable(const std::vector<EntityPtr>& entitys) : m_size(glm::vec3(5.f, HEIGHT_WALL, 5.f)) {
		for(const auto& entity : entitys) {
			insert(entity);
		}
	}

	void addRenderableGrid(std::vector<EntityPtr>& entitys) {
		/*for(float x = -20.f; x < 20.f; x += m_size.x) {
			for(float y = -20.f; y < 20.f; y += m_size.y) {
				for(float z = 0.f; z < 20.f; z += m_size.z) {
					EntityPtr cell = std::make_shared<Entity>();
					RenderablePtr<Box> box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"), 
																				glm::vec3(x, y, z),
																				m_size, 
																				glm::vec4(1.0f, 0.f, 0.f, 1.f));
					RenderableComponentPtr render = std::make_shared<RenderableComponent>();
					render->m_renderable = box;
					cell->addComponent<RenderableComponent>(render);
					entitys.push_back(cell);
				}
			}
		}*/
	}

	std::set<Partition, Comp> getPartitionIntersect(const Partition& p) {
		std::set<Partition, Comp> partitions;

		partitions.insert(getPartition(p.m_position));

		partitions.insert(getPartition(p.m_position + glm::vec3(p.m_size.x, 0.f, 0.f)));
		partitions.insert(getPartition(p.m_position + glm::vec3(0.f, p.m_size.y, 0.f)));
		partitions.insert(getPartition(p.m_position + glm::vec3(0.f, 0.f, p.m_size.z)));

		partitions.insert(getPartition(p.m_position + glm::vec3(p.m_size.x, p.m_size.y, 0.f)));
		partitions.insert(getPartition(p.m_position + glm::vec3(p.m_size.x, 0.f, p.m_size.z)));
		partitions.insert(getPartition(p.m_position + glm::vec3(0.f, p.m_size.y, p.m_size.z)));

		partitions.insert(getPartition(p.m_position + p.m_size));

		return partitions;
	}

	const Partition getPartition(const glm::vec3& p) {
		glm::vec3 pos(glm::floor(p.x / m_size.x),
					  glm::floor(p.y / m_size.y),
					  glm::floor(p.z / m_size.z));
		pos *= m_size;

		return Partition(pos, m_size);
	}
	
	private:
		std::map<Partition, std::set<EntityPtr>, Comp>	m_map;

		glm::vec3										m_size;
};

using HashMapCollisablePtr = std::shared_ptr<HashMapCollisable>;

class Collider {
	public:
		void run(std::vector<EntityPtr>& entitys) {
			std::vector<EntityPtr> MovableEntitys = EntityManager::getEntitysByComponent<Movable>(entitys);
			//std::cout << "esf " << MovableEntitys.size() << std::endl;
			for(auto& entity : MovableEntitys) {

				update_hashmap_movable(entity);

				if(entity->getComponent<CollidedComponent>() != nullptr) {
					continue;
				}

				MovablePtr movable = entity->getComponent<Movable>();

				//for(auto& entity_collision : m_hashmap->getNearestEntitys(entity)) {
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

		void setDungeon(DungeonPtr dungeon) {
			m_dungeon = dungeon;
		}

		void initialize_hashmap(std::vector<EntityPtr>& entitys) {
			m_hashmap = std::make_shared<HashMapCollisable>(entitys);
			m_hashmap->addRenderableGrid(entitys);
		}

		void update_hashmap_movable(const EntityPtr entity) {
			m_hashmap->erase(entity);
			m_hashmap->insert(entity);
		}
	private:
	
		DungeonPtr				m_dungeon;
		HashMapCollisablePtr	m_hashmap;
};