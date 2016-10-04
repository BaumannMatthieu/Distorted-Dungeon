#pragma once

#include <map>
#include <memory>

#include <SDL2/SDL.h>
#include "../include/MeshOBJRenderable.hpp"
#include "../include/ShaderProgram.hpp"

template<typename Type>
class Resources {
	public:
		Resources() {
		}

		~Resources() {
		}

		void add(const std::string& filename, const std::string& key) {
			if(m_map.find(key) != m_map.end()) {
				return;
			}

			std::shared_ptr<Type> value = load(filename);
			m_map.insert(std::make_pair(key, value));
		}

		std::shared_ptr<Type> get(const std::string& key) {
			if(m_map.find(key) == m_map.end()) {
				return nullptr;
			}

			return m_map[key];
		}

	private: 
		std::shared_ptr<Type> load(const std::string& filename);

		std::map<std::string, std::shared_ptr<Type>>		m_map;
};

using Texture = SDL_Surface;
using TexturePtr = std::shared_ptr<SDL_Surface>;

//using Sound = ...;

template<>
std::shared_ptr<Texture> Resources<Texture>::load(const std::string& filename);

template<>
std::shared_ptr<MeshOBJ> Resources<MeshOBJ>::load(const std::string& filename);

template<>
std::shared_ptr<ShaderProgram> Resources<ShaderProgram>::load(const std::string& filename);

extern Resources<Texture> textures;
extern Resources<MeshOBJ> meshes;
extern Resources<ShaderProgram> shaders;

