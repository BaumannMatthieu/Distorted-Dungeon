#include <iostream>

#include <SDL2/SDL.h>
#include "../include/Resource.hpp"
#include "../include/MeshOBJRenderable.hpp"
#include "../include/ShaderProgram.hpp"

template<typename Type>
std::shared_ptr<Type> Resources<Type>::load(const std::string& filename) {
	return nullptr;
}

template<>
std::shared_ptr<Texture> Resources<Texture>::load(const std::string& filename) {
	Texture* tex = SDL_LoadBMP(filename.c_str());
	if(tex == NULL) {
		std::cerr << "Error when loading the texture at : " << filename << std::endl;
		return nullptr;
	}

	return std::shared_ptr<Texture>(tex);
}

template<>
std::shared_ptr<MeshOBJ> Resources<MeshOBJ>::load(const std::string& filename) {
	MeshOBJPtr mesh = std::make_shared<MeshOBJ>(filename);

	if(mesh == nullptr) {
		std::cerr << "Error when loading the following .OBJ mesh : " << filename << std::endl;
	}

	return mesh;
}

template<>
std::shared_ptr<ShaderProgram> Resources<ShaderProgram>::load(const std::string& filename) {
	ShaderProgramPtr shader = std::make_shared<ShaderProgram>(filename + ".vert", filename + ".frag");

	if(shader == nullptr) {
		std::cerr << "Error when loading the following shader : " << filename << std::endl;
	}

	return shader;
}