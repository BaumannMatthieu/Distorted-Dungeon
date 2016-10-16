#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <limits>

#include <random>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/EntityManager.h"
#include "../include/Entity.h"

#include "../include/RenderableComponent.hpp"
#include "../include/CollisableComponent.hpp"

#include "../include/PlaneRenderable.hpp"
#include "../include/LineRenderable.hpp"

#include "../include/ShaderProgram.hpp"

template<typename DistributionSize, typename DistributionPosition>
class Room
{
	public:
		Room(std::default_random_engine& generator, float min_area = 5.f) {
			/* generating rooms which are good (does not seem like corridors) */
			float area = 0.f;

			float x_scale, z_scale;

			while(area < min_area) {
				DistributionSize distribution_x(3.0f, 0.5f);
				x_scale = distribution_x(generator);

				DistributionSize distribution_z(x_scale, 0.5f);
				z_scale = distribution_z(generator);

				x_scale = glm::round(x_scale);
				z_scale = glm::round(z_scale);

				if(z_scale <= 0.f||x_scale <= 0.f) {
					continue;
				}

				area = x_scale*z_scale;
			}

			m_size = glm::vec3(x_scale, 0, z_scale);

			float x_position, z_position;
			DistributionPosition distribution_x(0, 5);
			x_position = distribution_x(generator);

			DistributionPosition distribution_z(0, 5);
			z_position = distribution_z(generator);

			x_position = glm::round(x_position);
			z_position = glm::round(z_position);

			m_center = glm::vec3(x_position + 0.5f*x_scale, 0.f, z_position + 0.5f*z_scale);
		}

		~Room() {

		}
		
		const glm::vec3& getCenter() const {
			return m_center;
		}

		void setPosition(const glm::vec3& position) {
			m_center = position + 0.5f*m_size;
		}

		void translate(const glm::vec3& tr) {
			m_center += tr;
		}
		
		const glm::vec3 getPosition() const {
			return m_center - 0.5f*m_size;
		}
		
		const glm::vec3& getSize() const {
			return m_size;
		}

		bool operator==(const Room& room) {
			return (this->m_center == room.getCenter() && this->m_size == room.getSize());
		}

		const std::vector<std::pair<glm::vec2, glm::vec2>> getSegments() const {
			glm::vec3 position = getPosition();
			glm::vec2 pos(position.x, position.z);

			std::vector<std::pair<glm::vec2, glm::vec2>> segments;
			segments.push_back(std::make_pair(pos, pos + glm::vec2(m_size.x, 0.f)));
			segments.push_back(std::make_pair(pos + glm::vec2(m_size.x, 0.f), pos + glm::vec2(m_size.x, m_size.z)));
			segments.push_back(std::make_pair(pos + glm::vec2(m_size.x, m_size.z), pos + glm::vec2(0.f, m_size.z)));
			segments.push_back(std::make_pair(pos + glm::vec2(0.f, m_size.z), pos));

			return segments;
		}

	private:
		//#define SIZE_TILE 48
		//#define SIZE_TILE 8
		#define HEIGHT_WALL 2

		glm::vec3	m_center;
		glm::vec3	m_size;
};

class Dungeon {
	private:
		bool intersect(const Room<std::normal_distribution<float>, std::normal_distribution<float>>& room1,
					   const Room<std::normal_distribution<float>, std::normal_distribution<float>>& room2) {
			glm::vec3 r1_pos = room1.getPosition();
			glm::vec3 r2_pos = room2.getPosition();

			glm::vec3 r1_size = room1.getSize();
			glm::vec3 r2_size = room2.getSize();

			bool no_overlap = 
			   (r1_pos.x > r2_pos.x + r2_size.x)||
			   (r2_pos.x > r1_pos.x + r1_size.x)||
			   (r1_pos.z > r2_pos.z + r2_size.z)||
			   (r2_pos.z > r1_pos.z + r1_size.z);
				
			return !no_overlap;
		}

		bool intersect(float x1, float y1, float w1, float h1,
					   float x2, float y2, float w2, float h2) {
			bool no_overlap = 
			   (x1 >= x2 + w2)||
			   (x2 >= x1 + w1)||
			   (y1 >= y2 + h2)||
			   (y2 >= y1 + h1);
				
			return !no_overlap;
		}

		bool rooms_distribution_valid() {
			for(unsigned int i = 0; i < m_rooms.size(); ++i) {
				for(unsigned int j = i; j < m_rooms.size(); ++j) {
					if(i != j) {
						if(intersect(m_rooms[i], m_rooms[j])) {
							return false;
						}
					}
				}
			}
			return true;
		}
	public:
		static float manhattan_distance(const glm::vec3& p1, const glm::vec3& p2) {
			return glm::abs(p1.x - p2.x) + glm::abs(p1.y - p2.y) + glm::abs(p1.z - p2.z);
		}

		Dungeon(std::default_random_engine& generator, unsigned int num_rooms, EntityManager& entity_manager) {
			ShaderProgramPtr basic = std::make_shared<ShaderProgram>("../shaders/2d.vert", "../shaders/2d.frag");

			ShaderProgramPtr textured = std::make_shared<ShaderProgram>("../shaders/textured.vert", "../shaders/textured.frag");

			for(unsigned int i = 0; i < num_rooms; ++i) {
				m_rooms.push_back(Room<std::normal_distribution<float>, std::normal_distribution<float>>(generator, 5.f));
			}

			//float radius = 50.0f;
			unsigned int c = 0;
			while(!rooms_distribution_valid()) {
				for(auto& current_room : m_rooms) {
					std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>> intersect_rooms;
					for(auto& room : m_rooms) {
						if(intersect(room, current_room)) {
							intersect_rooms.push_back(room);
						}
					}
					unsigned int count = 0;
					glm::vec3 directional_room = glm::vec3(0.f);

					for(auto& room : intersect_rooms) {
						float d = glm::length(room.getPosition() - current_room.getPosition());
						
						if(d > 0.f) {
							count++;

							glm::vec3 diff = glm::normalize(room.getPosition() - current_room.getPosition());
							diff /= d;
							directional_room += diff;
						}
					}
					if(count != 0) {
						directional_room /= count;
					}

					current_room.translate(-directional_room);
				}
				c++;
			}

			for(auto& room : m_rooms) {
				glm::vec3 position = room.getPosition();
				glm::vec3 size = room.getSize();

				room.setPosition(glm::ceil(position));

				position = glm::ceil(position);

				/*std::cout << "room : "<< std::endl;
				std::cout << position.x << " " << position.y << " " << position.z << " " << std::endl;
				std::cout << size.x << " " << size.y << " " << size.z << " " << std::endl;*/
			}

			std::vector<unsigned> rooms_id;
			unsigned int inserted = 0;
			unsigned int num_rooms_selected = 20;
			while(inserted < num_rooms_selected) {
				unsigned int k = std::rand() % m_rooms.size();
				
				if(std::find(rooms_id.begin(), rooms_id.end(), k) == rooms_id.end()) {
					rooms_id.push_back(k);
					inserted++;

					EntityPtr entity = std::make_shared<Entity>();

					RenderablePtr<Plane> floor = std::make_shared<Renderable<Plane>>(textured, "stone");
					floor->scaleLocalMatrix(glm::vec3(m_rooms[k].getSize().x, 0.f, m_rooms[k].getSize().z));
					floor->translateLocalMatrix(glm::vec3(m_rooms[k].getCenter().x, 0.f, m_rooms[k].getCenter().z));

					floor->scaleTexCoords(glm::vec2(m_rooms[k].getSize().x, m_rooms[k].getSize().z));

					RenderableComponentPtr render_component = std::make_shared<RenderableComponent>();
					render_component->m_renderable = floor;
					
					entity->addComponent<RenderableComponent>(render_component);

					CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
					collisable->m_position = m_rooms[k].getPosition() - glm::vec3(0.0f, 1.0f, 0.0f);
					collisable->m_size = m_rooms[k].getSize() + glm::vec3(0.0f, 1.0f, 0.0f);
					collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
													  collisable->m_position,
													  collisable->m_size,
													  glm::vec4(0.f, 1.f, 0.f, 1.f));

					entity->addComponent<Collisable<Cobble>>(collisable);

					/*EntityPtr e = std::make_shared<Entity>();
					RenderableComponentPtr r = std::make_shared<RenderableComponent>();
					r->m_renderable = collisable->m_box;
					e->addComponent<RenderableComponent>(r);
					entity_manager.add(e);*/
					entity_manager.add(entity);

					std::vector<glm::vec3> position_colonnes(4);
					position_colonnes[0] = glm::vec3(m_rooms[k].getSize().x/2.f - 0.25f, 0.f, m_rooms[k].getSize().z/2.f - 0.25f);
					position_colonnes[1] = glm::vec3(m_rooms[k].getSize().x/2.f - 0.25f, 0.f, -m_rooms[k].getSize().z/2.f + 0.25f);
					position_colonnes[2] = glm::vec3(-m_rooms[k].getSize().x/2.f + 0.25f, 0.f, m_rooms[k].getSize().z/2.f - 0.25f);
					position_colonnes[3] = glm::vec3(-m_rooms[k].getSize().x/2.f + 0.25f, 0.f, -m_rooms[k].getSize().z/2.f + 0.25f);

					for(unsigned int i = 0; i < 4; ++i) {
						EntityPtr colonne = std::make_shared<Entity>();
						RenderablePtr<MeshOBJ> colonne_render = std::make_shared<Renderable<MeshOBJ>>(shaders.get("textured"), "colonne", "colonne_tex");

						colonne_render->scaleLocalMatrix(glm::vec3(0.25f, 0.25f, 0.25f));
						colonne_render->translateHeritanceMatrix(glm::vec3(m_rooms[k].getCenter().x, 0.f, m_rooms[k].getCenter().z) + position_colonnes[i]);
						
						RenderableComponentPtr render = std::make_shared<RenderableComponent>();
						render->m_renderable = colonne_render;
						colonne->addComponent<RenderableComponent>(render);

						CollisablePtr<Cobble> col = std::make_shared<Collisable<Cobble>>();

						//collisable->m_position_start = collisable->m_position;
						col->m_size = getSizePositions(colonne_render, 0.25f);
						col->m_position = glm::vec3(m_rooms[k].getCenter().x, 0.f, m_rooms[k].getCenter().z) + position_colonnes[i] - col->m_size/2.f;
						col->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																			  col->m_position,
																			  col->m_size,
																			  glm::vec4(1.f, 1.f, 1.f, 1.f));
						colonne->addComponent<Collisable<Cobble>>(col);

						entity_manager.add(colonne);
					}
				}
			}

			/*for(unsigned int i = 0; i < rooms_id.size(); ++i) {
				for(unsigned int j = i + 1; j < rooms_id.size(); ++j) {
					EntityPtr entity = std::make_shared<Entity>();

					RenderablePtr<Line> line = std::make_shared<Renderable<Line>>(basic, m_rooms[rooms_id[i]].getCenter(), m_rooms[rooms_id[j]].getCenter());

					RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
					renderable_component->m_renderable = line;
					
					entity->addComponent<RenderableComponent>(renderable_component);

					entity_manager.add(entity);
				}
			}*/

			// Minimum tree of distance
			std::vector<unsigned int> tree;
			std::vector<std::pair<unsigned int, unsigned int>> edges;
			tree.push_back(rooms_id[0]);
			while(nearest_tree(rooms_id, tree, edges));

			/*for(unsigned int i = 0; i < edges.size(); ++i) {
				EntityPtr entity = std::make_shared<Entity>();

				unsigned int k = edges[i].first;
				unsigned int v = edges[i].second;

				//glm::vec3 scale(SIZE_TILE, 0.f, SIZE_TILE);

				RenderablePtr<Line> line = std::make_shared<Renderable<Line>>(basic,
																			 glm::vec3(m_rooms[k].getCenter().x, 0.f, m_rooms[k].getCenter().z),
																			 glm::vec3(m_rooms[v].getCenter().x, 0.f, m_rooms[v].getCenter().z),
																			 glm::vec4(1.f, 0.f, 0.f, 1.f),
																			 glm::vec4(1.f, 0.f, 0.f, 1.f));

				RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
				renderable_component->m_renderable = line;
				
				entity->addComponent<RenderableComponent>(renderable_component);

				entity_manager.add(entity);
			}*/

			// Computation of corridors between nodes of minimum tree
			std::vector<std::pair<glm::vec2, glm::vec2>> corridors;
			for(unsigned int i = 0; i < edges.size(); ++i) {
				unsigned int k = edges[i].first;
				unsigned int v = edges[i].second;

				std::vector<std::pair<glm::vec2, glm::vec2>> segments_r1 = m_rooms[k].getSegments();
				std::vector<std::pair<glm::vec2, glm::vec2>> segments_r2 = m_rooms[v].getSegments();
				std::pair<glm::vec2, glm::vec2> segment_inter_r1;
				std::pair<glm::vec2, glm::vec2> segment_inter_r2;

				glm::vec2 r1_center(m_rooms[k].getCenter().x, m_rooms[k].getCenter().z);
				glm::vec2 r2_center(m_rooms[v].getCenter().x, m_rooms[v].getCenter().z);
				unsigned int o = 0;
				for(unsigned int i = 0; i < 4; ++i) {
					if(intersect(r1_center, r2_center, segments_r1[i].first, segments_r1[i].second)) {
						segment_inter_r1 = segments_r1[i];
					}

					if(intersect(r1_center, r2_center, segments_r2[i].first, segments_r2[i].second)) {
						segment_inter_r2 = segments_r2[i];
						o = i;
					}
				}

				computeCorridor(segment_inter_r1, segment_inter_r2, corridors);

				if(vertical(segment_inter_r1) && vertical(segment_inter_r2)) {
					float y_inter;
					if(!intersect(segment_inter_r1.first.y, segment_inter_r1.second.y, segment_inter_r2.first.y, segment_inter_r2.second.y, y_inter)) {
						glm::vec2 i;
						if(intersect(r1_center, r2_center, segment_inter_r2.first, segment_inter_r2.second, i)) {
							float dist_first = glm::length(segment_inter_r2.first - i);
							float dist_second = glm::length(segment_inter_r2.second - i);

							if(dist_first < dist_second) {
								segment_inter_r2 = segments_r2[(o - 1) % segments_r2.size()];
							} else {
								segment_inter_r2 = segments_r2[(o + 1) % segments_r2.size()];
							}

							computeCorridor(segment_inter_r1, segment_inter_r2, corridors);
						}
					}
				} else if(horizontal(segment_inter_r1) && horizontal(segment_inter_r2)) {
					float x_inter;
					if(!intersect(segment_inter_r1.first.x, segment_inter_r1.second.x, segment_inter_r2.first.x, segment_inter_r2.second.x, x_inter)) {
						glm::vec2 i;
						if(intersect(r1_center, r2_center, segment_inter_r2.first, segment_inter_r2.second, i)) {
							float dist_first = glm::length(segment_inter_r2.first - i);
							float dist_second = glm::length(segment_inter_r2.second - i);

							if(dist_first < dist_second) {
								segment_inter_r2 = segments_r2[(o - 1) % segments_r2.size()];
							} else {
								segment_inter_r2 = segments_r2[(o + 1) % segments_r2.size()];
							}

							computeCorridor(segment_inter_r1, segment_inter_r2, corridors);
						}
					}
				}
			}

			std::vector<std::pair<int, int>> tiles_corridors;

			for(unsigned int i = 0; i < corridors.size(); ++i) {
				if(horizontal(corridors[i])) {
					float x_first = glm::min(corridors[i].first.x, corridors[i].second.x);
					float x_second =  glm::max(corridors[i].first.x, corridors[i].second.x);
					int y = glm::floor(corridors[i].first.y);

					for(int x = x_first; x <= glm::floor(x_second); ++x) {
						bool add_tile = true;
						for(unsigned int j = 0; j < rooms_id.size(); ++j) {
							int index = rooms_id[j];
							if(intersect(x, y, 1, 1, m_rooms[index].getPosition().x, m_rooms[index].getPosition().z, m_rooms[index].getSize().x, m_rooms[index].getSize().z)) {
								add_tile = false;
								break;
							}
						}
						if(add_tile)
							tiles_corridors.push_back(std::make_pair(x, y));
					}	
				} else if(vertical(corridors[i])) {
					float y_first = glm::min(corridors[i].first.y, corridors[i].second.y);
					float y_second =  glm::max(corridors[i].first.y, corridors[i].second.y);
					int x = glm::floor(corridors[i].first.x);

					for(int y = y_first; y <= glm::floor(y_second); ++y) {
						bool add_tile = true;
						for(unsigned int j = 0; j < rooms_id.size(); ++j) {
							int index = rooms_id[j];
							if(intersect(x, y, 1, 1, m_rooms[index].getPosition().x, m_rooms[index].getPosition().z, m_rooms[index].getSize().x, m_rooms[index].getSize().z)) {
								add_tile = false;
								break;
							}
						}
						if(add_tile)
							tiles_corridors.push_back(std::make_pair(x, y));
					}
				}
			}

			for(unsigned int j = 0; j < tiles_corridors.size(); ++j) {
				EntityPtr entity = std::make_shared<Entity>();

				RenderablePtr<Plane> tile_render = std::make_shared<Renderable<Plane>>(textured, "stone");
				//glm::vec3 scale(SIZE_TILE, 0.f, SIZE_TILE);
				glm::vec3 translate(tiles_corridors[j].first, 0.f, tiles_corridors[j].second);
				
				//tile->scaleLocalMatrix(glm::vec3(1.f));
				tile_render->scaleLocalMatrix(glm::vec3(1.0f, 0.f, 1.0f));
				tile_render->translateLocalMatrix(translate + glm::vec3(0.5f, 0.f, 0.5f));
				//tile_render->scaleTexCoords(glm::vec2(1.0f));

				RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
				renderable_component->m_renderable = tile_render;
				
				entity->addComponent<RenderableComponent>(renderable_component);

				CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
				collisable->m_position = glm::vec3(tiles_corridors[j].first, -1.0f, tiles_corridors[j].second);
				collisable->m_size = glm::vec3(1.f, 1.f, 1.f);
				collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																	  collisable->m_position,
																	  collisable->m_size,
																	  glm::vec4(0.f, 1.f, 0.f, 1.f));
				entity->addComponent<Collisable<Cobble>>(collisable);

				entity_manager.add(entity);
			}

			/*for(unsigned int j = 0; j < corridors.size(); ++j) {
				EntityPtr entity = std::make_shared<Entity>();
				
				//glm::vec3 scale(SIZE_TILE, 0.f, SIZE_TILE);

				RenderablePtr<Line> line = std::make_shared<Renderable<Line>>(basic,
																			 glm::vec3(corridors[j].first.x, 0.f, corridors[j].first.y),
																			 glm::vec3(corridors[j].second.x, 0.f, corridors[j].second.y),
																			 glm::vec4(0.f, 0.f, 1.f, 1.f),
																			 glm::vec4(0.f, 0.f, 1.f, 1.f));

				RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
				renderable_component->m_renderable = line;
				
				entity->addComponent<RenderableComponent>(renderable_component);

				entity_manager.add(entity);
			}*/

			// Walls around rooms generation
			for(int index : rooms_id) {
				m_rooms_dungeon.push_back(m_rooms[index]);
			}

			for(auto& room : m_rooms_dungeon) {
				glm::vec2 position(room.getPosition().x, room.getPosition().z);
				glm::vec2 size(room.getSize().x, room.getSize().z);

				for(int x = position.x; x < position.x + size.x; ++x) {
					if(wall_possible(x, position.y, m_rooms_dungeon, tiles_corridors, TOP)) {
						EntityPtr entity = std::make_shared<Entity>();

						RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
						glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
						glm::vec3 translate(x, 0.f, position.y);
						
						wall->scaleLocalMatrix(scale);
						wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
						wall->translateLocalMatrix(glm::vec3(0.5f, HEIGHT_WALL*0.5f, 0.f));
						wall->translateLocalMatrix(translate);

						wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

						RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
						renderable_component->m_renderable = wall;
						
						entity->addComponent<RenderableComponent>(renderable_component);

						/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
						collisable->m_position = glm::vec2(x, position.y - 1);
						collisable->m_size = glm::vec2(1.f, 1.f);

						entity->addComponent<Collisable<Square>>(collisable);*/

						CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
						collisable->m_position = glm::vec3(x, 0.0f, position.y - 1);
						collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
						collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																			  collisable->m_position,
																			  collisable->m_size,
																			  glm::vec4(0.f, 1.f, 0.f, 1.f));
						entity->addComponent<Collisable<Cobble>>(collisable);

						entity_manager.add(entity);
					}

					if(wall_possible(x, position.y + size.y - 1, m_rooms_dungeon, tiles_corridors, BOTTOM)) {
						EntityPtr entity = std::make_shared<Entity>();

						RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
						glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
						glm::vec3 translate(x, 0.f, position.y + size.y);
						
						wall->scaleLocalMatrix(scale);
						wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
						wall->translateLocalMatrix(glm::vec3(0.5f, HEIGHT_WALL*0.5f, 0.f));
						wall->translateLocalMatrix(translate);

						wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

						RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
						renderable_component->m_renderable = wall;
						
						entity->addComponent<RenderableComponent>(renderable_component);

						/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
						collisable->m_position = glm::vec2(x, position.y + size.y);
						collisable->m_size = glm::vec2(1.f, 1.f);

						entity->addComponent<Collisable<Square>>(collisable);*/

						CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
						collisable->m_position = glm::vec3(x, 0.0f, position.y + size.y);
						collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
						collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																			  collisable->m_position,
																			  collisable->m_size,
																			  glm::vec4(0.f, 1.f, 0.f, 1.f));
						entity->addComponent<Collisable<Cobble>>(collisable);

						entity_manager.add(entity);
					}
				}

				for(int y = position.y; y < position.y + size.y; ++y) {
					if(wall_possible(position.x + size.x - 1, y, m_rooms_dungeon, tiles_corridors, RIGHT)) {
						EntityPtr entity = std::make_shared<Entity>();

						RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
						glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
						glm::vec3 translate(position.x + size.x, 0.f, y);
						
						wall->scaleLocalMatrix(scale);
						wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
						wall->rotateLocalMatrix(90, glm::vec3(0, 1, 0));
						wall->translateLocalMatrix(glm::vec3(0.f, HEIGHT_WALL*0.5f, 0.5f));
						wall->translateLocalMatrix(translate);

						wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

						RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
						renderable_component->m_renderable = wall;
						
						entity->addComponent<RenderableComponent>(renderable_component);

						/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
						collisable->m_position = glm::vec2(position.x + size.x, y);
						collisable->m_size = glm::vec2(1.f, 1.f);

						entity->addComponent<Collisable<Square>>(collisable);*/

						CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
						collisable->m_position = glm::vec3(position.x + size.x, 0.0f, y);
						collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
						collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																			  collisable->m_position,
																			  collisable->m_size,
																			  glm::vec4(0.f, 1.f, 0.f, 1.f));
						entity->addComponent<Collisable<Cobble>>(collisable);

						entity_manager.add(entity);
					}
				}

				for(int y = position.y; y < position.y + size.y; ++y) {
					if(wall_possible(position.x, y, m_rooms_dungeon, tiles_corridors, LEFT)) {
						EntityPtr entity = std::make_shared<Entity>();

						RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
						glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
						glm::vec3 translate(position.x, 0.f, y);
						
						wall->scaleLocalMatrix(scale);
						wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
						wall->rotateLocalMatrix(90, glm::vec3(0, 1, 0));
						wall->translateLocalMatrix(glm::vec3(0.f, HEIGHT_WALL*0.5f, 0.5f));
						wall->translateLocalMatrix(translate);

						wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

						RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
						renderable_component->m_renderable = wall;
						
						entity->addComponent<RenderableComponent>(renderable_component);

						/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
						collisable->m_position = glm::vec2(position.x - 1, y);
						collisable->m_size = glm::vec2(1.f, 1.f);

						entity->addComponent<Collisable<Square>>(collisable);*/

						CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
						collisable->m_position = glm::vec3(position.x - 1, 0.0f, y);
						collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
						collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																			  collisable->m_position,
																			  collisable->m_size,
																			  glm::vec4(0.f, 1.f, 0.f, 1.f));
						entity->addComponent<Collisable<Cobble>>(collisable);

						entity_manager.add(entity);
					}
				}
			}

			for(auto& tile : tiles_corridors) {
				if(wall_possible(tile.first, tile.second, m_rooms_dungeon, tiles_corridors, TOP)) {
					EntityPtr entity = std::make_shared<Entity>();

					RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
					glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
					glm::vec3 translate(tile.first, 0.f, tile.second);
					
					wall->scaleLocalMatrix(scale);
					wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
					wall->translateLocalMatrix(glm::vec3(0.5f, HEIGHT_WALL*0.5f, 0.f));
					wall->translateLocalMatrix(translate);

					wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

					RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
					renderable_component->m_renderable = wall;
					
					entity->addComponent<RenderableComponent>(renderable_component);

					/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
					collisable->m_position = glm::vec2(tile.first, tile.second - 1);
					collisable->m_size = glm::vec2(1.f, 1.f);

					entity->addComponent<Collisable<Square>>(collisable);*/

					CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
					collisable->m_position = glm::vec3(tile.first, 0.0f, tile.second - 1);
					collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
					collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																		  collisable->m_position,
																		  collisable->m_size,
																		  glm::vec4(0.f, 1.f, 0.f, 1.f));
					entity->addComponent<Collisable<Cobble>>(collisable);


					entity_manager.add(entity);
				}

				if(wall_possible(tile.first, tile.second, m_rooms_dungeon, tiles_corridors, BOTTOM)) {
					EntityPtr entity = std::make_shared<Entity>();

					RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
					glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
					glm::vec3 translate(tile.first, 0.f, tile.second + 1);
					
					wall->scaleLocalMatrix(scale);
					wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
					wall->translateLocalMatrix(glm::vec3(0.5f, HEIGHT_WALL*0.5f, 0.f));
					wall->translateLocalMatrix(translate);

					wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

					RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
					renderable_component->m_renderable = wall;
					
					entity->addComponent<RenderableComponent>(renderable_component);

					/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
					collisable->m_position = glm::vec2(tile.first, tile.second + 1);
					collisable->m_size = glm::vec2(1.f, 1.f);

					entity->addComponent<Collisable<Square>>(collisable);*/

					CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
					collisable->m_position = glm::vec3(tile.first, 0.0f, tile.second + 1);
					collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
					collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																		  collisable->m_position,
																		  collisable->m_size,
																		  glm::vec4(0.f, 1.f, 0.f, 1.f));
					entity->addComponent<Collisable<Cobble>>(collisable);

					entity_manager.add(entity);
				}
			
				if(wall_possible(tile.first, tile.second, m_rooms_dungeon, tiles_corridors, RIGHT)) {
					EntityPtr entity = std::make_shared<Entity>();

					RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
					glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
					glm::vec3 translate(tile.first + 1, 0.f, tile.second);
					
					wall->scaleLocalMatrix(scale);
					wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
					wall->rotateLocalMatrix(90, glm::vec3(0, 1, 0));
					wall->translateLocalMatrix(glm::vec3(0.f, HEIGHT_WALL*0.5f, 0.5f));
					wall->translateLocalMatrix(translate);

					wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

					RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
					renderable_component->m_renderable = wall;
					
					entity->addComponent<RenderableComponent>(renderable_component);

					/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
					collisable->m_position = glm::vec2(tile.first + 1, tile.second);
					collisable->m_size = glm::vec2(1.f, 1.f);

					entity->addComponent<Collisable<Square>>(collisable);*/

					CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
					collisable->m_position = glm::vec3(tile.first + 1, 0.0f, tile.second);
					collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
					collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																		  collisable->m_position,
																		  collisable->m_size,
																		  glm::vec4(0.f, 1.f, 0.f, 1.f));
					entity->addComponent<Collisable<Cobble>>(collisable);

					entity_manager.add(entity);
				}

				if(wall_possible(tile.first, tile.second, m_rooms_dungeon, tiles_corridors, LEFT)) {
					EntityPtr entity = std::make_shared<Entity>();

					RenderablePtr<Plane> wall = std::make_shared<Renderable<Plane>>(textured, "wall");
					glm::vec3 scale(1.0f, 0.f, HEIGHT_WALL);
					glm::vec3 translate(tile.first, 0.f, tile.second);
					
					wall->scaleLocalMatrix(scale);
					wall->rotateLocalMatrix(90, glm::vec3(1, 0, 0));
					wall->rotateLocalMatrix(90, glm::vec3(0, 1, 0));
					wall->translateLocalMatrix(glm::vec3(0.f, HEIGHT_WALL*0.5f, 0.5f));
					wall->translateLocalMatrix(translate);

					wall->scaleTexCoords(glm::vec2(1.0f, HEIGHT_WALL));

					RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
					renderable_component->m_renderable = wall;
					
					entity->addComponent<RenderableComponent>(renderable_component);

					/*CollisablePtr<Square> collisable = std::make_shared<Collisable<Square>>();
					collisable->m_position = glm::vec2(tile.first - 1, tile.second);
					collisable->m_size = glm::vec2(1.f, 1.f);

					entity->addComponent<Collisable<Square>>(collisable);*/

					CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
					collisable->m_position = glm::vec3(tile.first - 1, 0.0f, tile.second);
					collisable->m_size = glm::vec3(1.f, HEIGHT_WALL, 1.f);
					collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																		  collisable->m_position,
																		  collisable->m_size,
																		  glm::vec4(0.f, 1.f, 0.f, 1.f));
					entity->addComponent<Collisable<Cobble>>(collisable);

					entity_manager.add(entity);
				}
			}
		}

		enum {TOP, BOTTOM, RIGHT, LEFT};

		bool wall_possible(int x, int y,
						const std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>>& rooms_dungeon,
						const std::vector<std::pair<int, int>>& tiles_corridors,
						int direction) {
			bool wall = true;
			for(auto& room : rooms_dungeon) {
				glm::vec2 position(room.getPosition().x, room.getPosition().z);
				glm::vec2 size(room.getSize().x, room.getSize().z);

				switch(direction) {
					case TOP:
						if(y == position.y + size.y && x >= position.x && x < position.x + size.x) {
							wall = false;
						}
						break;
					case BOTTOM:
						if(y + 1 == position.y && x >= position.x && x < position.x + size.x) {
							wall = false;
						}
						break;
					case RIGHT:
						if(x + 1 == position.x && y >= position.y && y < position.y + size.y) {
							wall = false;
						}
						break;
					case LEFT:
						if(x == position.x + size.x && y >= position.y && y < position.y + size.y) {
							wall = false;
						}
						break;
					default:
						break;
				};

				if(!wall) {
					return false;
				}
			}

			for(auto& tile : tiles_corridors) {
				glm::vec2 position(tile.first, tile.second);
				glm::vec2 size(1, 1);

				switch(direction) {
					case TOP:
						if(y == position.y + size.y && x >= position.x && x < position.x + size.x) {
							wall = false;
						}
						break;
					case BOTTOM:
						if(y + 1 == position.y && x >= position.x && x < position.x + size.x) {
							wall = false;
						}
						break;
					case RIGHT:
						if(x + 1 == position.x && y >= position.y && y < position.y + size.y) {
							wall = false;
						}
						break;
					case LEFT:
						if(x == position.x + size.x && y >= position.y && y < position.y + size.y) {
							wall = false;
						}
						break;
					default:
						break;
				};

				if(!wall) {
					return false;
				}
			}

			return true;
		}


		void computeCorridor(const std::pair<glm::vec2, glm::vec2>& segment_inter_r1, const std::pair<glm::vec2, glm::vec2>& segment_inter_r2, std::vector<std::pair<glm::vec2, glm::vec2>>& corridors) {
			if(horizontal(segment_inter_r1) && horizontal(segment_inter_r2)) {
				float x_inter;
				if(intersect(segment_inter_r1.first.x, segment_inter_r1.second.x, segment_inter_r2.first.x, segment_inter_r2.second.x, x_inter)) {
					corridors.push_back(std::make_pair(glm::vec2(x_inter, segment_inter_r1.first.y), glm::vec2(x_inter, segment_inter_r2.first.y)));
				}
			} else if(vertical(segment_inter_r1) && vertical(segment_inter_r2)) {
				float y_inter;
				if(intersect(segment_inter_r1.first.y, segment_inter_r1.second.y, segment_inter_r2.first.y, segment_inter_r2.second.y, y_inter)) {
					corridors.push_back(std::make_pair(glm::vec2(segment_inter_r1.first.x, y_inter), glm::vec2(segment_inter_r2.first.x, y_inter)));
				}
			} else {
				if(horizontal(segment_inter_r1) && vertical(segment_inter_r2)) {
					float x_milieu_r1 = glm::floor((segment_inter_r1.first.x + segment_inter_r1.second.x)*0.5f);
					float y_milieu_r2 = glm::floor((segment_inter_r2.first.y + segment_inter_r2.second.y)*0.5f);

					corridors.push_back(std::make_pair(glm::vec2(x_milieu_r1, segment_inter_r1.first.y), glm::vec2(x_milieu_r1, y_milieu_r2)));
					corridors.push_back(std::make_pair(glm::vec2(x_milieu_r1, y_milieu_r2), glm::vec2(segment_inter_r2.first.x, y_milieu_r2)));
				}
				if(vertical(segment_inter_r1) && horizontal(segment_inter_r2)) {
					float y_milieu_r1 = glm::floor((segment_inter_r1.first.y + segment_inter_r1.second.y)*0.5f);
					float x_milieu_r2 = glm::floor((segment_inter_r2.first.x + segment_inter_r2.second.x)*0.5f);

					corridors.push_back(std::make_pair(glm::vec2(segment_inter_r1.first.x, y_milieu_r1), glm::vec2(x_milieu_r2, y_milieu_r1)));
					corridors.push_back(std::make_pair(glm::vec2(x_milieu_r2, y_milieu_r1), glm::vec2(x_milieu_r2, segment_inter_r2.first.y)));
				}
			}
		}

		bool intersect(const glm::vec2& s1_p1, const glm::vec2& s1_p2, const glm::vec2& s2_p1, const glm::vec2& s2_p2) {
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

		bool intersect(const glm::vec2& s1_p1, const glm::vec2& s1_p2, const glm::vec2& s2_p1, const glm::vec2& s2_p2, glm::vec2& i) {
			glm::vec2 r = s1_p2 - s1_p1;
			glm::vec2 s = s2_p2 - s2_p1;

			glm::vec2 p = s1_p1;
			glm::vec2 q = s2_p1;

			float cross = r.x*s.y - r.y*s.x;

			if(cross == 0)
				return false;

			float t = ((q.x - p.x)*s.y - (q.y - p.y)*s.x)/cross;
			float u = ((q.x - p.x)*r.y - (q.y - p.y)*r.x)/cross;

			if(t >= 0.f && t <= 1.f && u >= 0.f && u <= 1.f) {
				i = p + t*r;
			}

			return true; 
		}

		bool horizontal(const std::pair<glm::vec2, glm::vec2>& s) {
			return (s.first.y == s.second.y);
		}

		bool vertical(const std::pair<glm::vec2, glm::vec2>& s) {
			return (s.first.x == s.second.x);
		}

		bool intersect(float x1, float x2, float y1, float y2, float& x_inter) {
			float tmp = std::min(x1, x2);
			x2 = std::max(x1, x2);
			x1 = tmp;

			tmp = std::min(y1, y2);
			y2 = std::max(y1, y2);
			y1 = tmp;

			if(x2 > y1 && y2 > x1) {
				float p1, p2;
				if(x1 >= y1) {
					p1 = x1;
				} else {
					p1 = y1;
				}
				if(x2 <= y2) {
					p2 = x2;
				} else {
					p2 = y2;
				}
				/*if(glm::abs(p1 - p2) < SIZE_TILE) {
					return false;
				}*/

				x_inter = (p2 + p1)*0.5f;

				return true;
			}

			return false;
		}

		bool nearest_tree(const std::vector<unsigned int>& indexes, std::vector<unsigned int>& points, std::vector<std::pair<unsigned int, unsigned int>>& edges) {
			if(edges.size() == indexes.size() - 1) {
				return false;
			}

			float distance_min = std::numeric_limits<float>::infinity();
			int k, v;

			for(unsigned int i = 0; i < indexes.size(); ++i) {
				if(std::find(points.begin(), points.end(), indexes[i]) == points.end()) {
					std::pair<unsigned int, float> distance = min_distance_point_to_tree(points, indexes[i]);
					if(distance.second < distance_min) {
						distance_min = distance.second;
						k = indexes[i];
						v = distance.first;
					}
				}
			}

			points.push_back(k);
			edges.push_back(std::make_pair(k, v));

			return true;
		}

		std::pair<unsigned int, float> min_distance_point_to_tree(const std::vector<unsigned int>& points, unsigned int index) {
			float min_distance = std::numeric_limits<float>::infinity();
			unsigned int p;
			for(auto point : points) {
				float dist = glm::length(m_rooms[index].getCenter() - m_rooms[point].getCenter());
				if(dist < min_distance) {
					min_distance = dist;
					p = point;
				}
			}

			return std::make_pair(p, min_distance);
		}

		~Dungeon() {

		}

		const std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>>& getRooms() const {
			return m_rooms_dungeon;
		}

	private:
		std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>> m_rooms;
		std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>> m_rooms_dungeon;
};

using DungeonPtr = std::shared_ptr<Dungeon>;
