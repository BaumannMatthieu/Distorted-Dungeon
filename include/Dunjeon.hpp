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
		Dungeon(std::default_random_engine& generator, unsigned int num_rooms, EntityManager& entity_manager);

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
