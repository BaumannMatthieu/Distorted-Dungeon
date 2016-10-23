#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <limits>

#include <random>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Dunjeon.hpp"

#include "../include/MovableComponent.hpp"
#include "../include/Physicable.hpp"
#include "../include/MotionSystem.hpp"

Dungeon::Dungeon(std::default_random_engine& generator, unsigned int num_rooms, EntityManager& entity_manager) {
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
			unsigned int num_rooms_selected = 5;
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

						//colonne_render->scaleLocalMatrix(glm::vec3(1.0f, 0.f, 1.f));
						colonne_render->scaleLocalMatrix(glm::vec3(0.25f, 0.25f, 0.25f));
						colonne_render->translateHeritanceMatrix(glm::vec3(m_rooms[k].getCenter().x, 0.f, m_rooms[k].getCenter().z) + position_colonnes[i]);
						
						RenderableComponentPtr render = std::make_shared<RenderableComponent>();
						render->m_renderable = colonne_render;
						colonne->addComponent<RenderableComponent>(render);

						CollisablePtr<Cobble> col = std::make_shared<Collisable<Cobble>>();

						//collisable->m_position_start = collisable->m_position;
						col->m_size = getSizePositions(colonne_render, 0.25f);
						col->m_position = glm::vec3(m_rooms[k].getCenter().x, 0.f, m_rooms[k].getCenter().z) + position_colonnes[i] - glm::vec3(1.f, 0.f, 1.f)*col->m_size/2.f;
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
				
				//tile_render->scaleLocalMatrix(glm::vec3(1.f));
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
				/*if(j % 5 == 0) {
					MovablePtr movable = std::make_shared<Movable>();

					movable->m_direction = glm::vec3(glm::pow(-1.f, static_cast<float>(j)), 0.f, 0.f);
					movable->m_position = glm::vec3(translate);
					movable->m_speed = 0.5f;
					entity->addComponent<Movable>(movable);

					PhysicPtr physic = std::make_shared<Physic>();
					physic->m_gravity = 9.81f;
					physic->m_mass = 0.f;
					physic->m_jump = false;
					entity->addComponent<Physic>(physic);

					MotionPtr<LineUpDown> motion = std::make_shared<Motion<LineUpDown>>();
					motion->m_first = glm::vec3(translate) - glm::vec3(1.f, 0.f, 0.f);
					motion->m_second = glm::vec3(translate) + glm::vec3(1.f, 0.f, 0.f);
					motion->m_first_to_second = std::rand() % 2;
					entity->addComponent<Motion<LineUpDown>>(motion);
				}*/

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