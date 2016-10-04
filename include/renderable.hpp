#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/ShaderProgram.hpp"
#include "../include/LineRenderable.hpp"
#include "../include/PlaneRenderable.hpp"
#include "../include/MeshOBJRenderable.hpp"
#include "../include/Resource.hpp"

#define SIZE_TILE 48

class Drawable;
using DrawablePtr = std::shared_ptr<Drawable>;

class Drawable : public std::enable_shared_from_this<Drawable> {
	public:
		Drawable(const ShaderProgramPtr shader) : m_shader(shader),
												  m_model(glm::mat4(1.0f)),
												  m_local(glm::mat4(1.0f)),
												  m_heritance(glm::mat4(1.0f)),
												  m_parent(nullptr) {
		}
		virtual ~Drawable() {}

		virtual void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int colorsAttributeIndex = m_shader->getAttributeLocation("in_color");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
			// Enable the attribute colors within the current VAO
			glEnableVertexAttribArray(colorsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glVertexAttribPointer(colorsAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);
			
			DrawArrays();

			glDisableVertexAttribArray(positionsAttributeIndex);
			glDisableVertexAttribArray(colorsAttributeIndex);
		}

		void setParent(const DrawablePtr parent) {
			m_parent = parent;
			m_parent->m_children.push_back(shared_from_this());
			computeModelMatrix();
		}

		void setLocalMatrix(const glm::mat4& model) {
			m_local = model;
			computeModelMatrix();
		}

		void scaleLocalMatrix(const glm::vec3& vector) {
			m_local = glm::scale(glm::mat4(1.0f), vector)*m_local;
			computeModelMatrix();
		}

		void rotateLocalMatrix(float angle, const glm::vec3& vector) {
			m_local = glm::rotate(glm::mat4(1.0f), angle, vector)*m_local;
			computeModelMatrix();
		}
		
		void translateLocalMatrix(const glm::vec3& vector) {
			m_local = glm::translate(glm::mat4(1.0f), vector)*m_local;
			computeModelMatrix();
		}

		void setHeritanceMatrix(const glm::mat4& model) {
			m_heritance = model;
			computeModelMatrix();
		}

		void scaleHeritanceMatrix(const glm::vec3& vector) {
			m_heritance = glm::scale(glm::mat4(1.0f), vector)*m_heritance;
			computeModelMatrix();
		}

		void rotateHeritanceMatrix(float angle, const glm::vec3& vector) {
			m_heritance = glm::rotate(glm::mat4(1.0f), angle, vector)*m_heritance;
			computeModelMatrix();
		}
		
		void translateHeritanceMatrix(const glm::vec3& vector) {
			m_heritance = glm::translate(glm::mat4(1.0f), vector)*m_heritance;
			computeModelMatrix();
		}

		const ShaderProgramPtr getShaderProgram() const {
			return m_shader;
		}

		virtual void DrawArrays() = 0;
	
	protected:
		void computeModelMatrix() {
			m_model = getParentModelMatrix() * m_local;
		}

		const glm::mat4 getParentModelMatrix() {
			if(m_parent == nullptr) {
				return m_heritance;
			}

			return m_parent->getParentModelMatrix() * m_heritance;
		}

	protected:
		std::vector<glm::vec3>		m_positions;
		std::vector<glm::vec4>		m_colors;
		
		GLuint						m_vertex_buffer_positions_id;
		GLuint						m_vertex_buffer_colors_id;

		ShaderProgramPtr			m_shader;

		glm::mat4 					m_local;
		glm::mat4 					m_heritance;

		glm::mat4 					m_model;

		std::vector<DrawablePtr> 	m_children;
		DrawablePtr				 	m_parent;
};



static GLuint CreateBufferTexture(const std::string& key) {
	GLuint textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	TexturePtr texture_data = textures.get(key);
	if(texture_data == nullptr) {
		std::cerr << "Error when loading texture : " << key << std::endl;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, 3, texture_data->w, texture_data->h, 0, GL_BGR, GL_UNSIGNED_BYTE, texture_data->pixels);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}


template<typename Primitive>
class Renderable : public Drawable {
	public:
		struct Material {
			float shininess;
		};
		using Material = struct Material;

	public:
		Renderable(const ShaderProgramPtr shader,
				   const std::string& key_texture) : Drawable(shader) {
			Primitive::initialize(m_positions,
								  m_normals,
								  m_colors,
								  m_tex_coords);

			// Material
			m_material.shininess = 0.5f;

			// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_normals_id);
			//glGenBuffers(1, &m_vertex_buffer_colors_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);
			// Positions
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_positions.size() * sizeof(glm::vec3),
						 m_positions.data(),
						 GL_STATIC_DRAW);

			// Normals
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_normals_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_normals.size() * sizeof(glm::vec3),
						 m_normals.data(),
						 GL_STATIC_DRAW);

			// Colors
			/*glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_colors.size() * sizeof(glm::vec4),
						 m_colors.data(),
						 GL_STATIC_DRAW);*/

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_textureID = CreateBufferTexture(key_texture);
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_normals_id);
			glDeleteBuffers(1, &m_vertex_buffer_colors_id);
			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
		}

		void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int normalsAttributeIndex = m_shader->getAttributeLocation("in_normal");
			//int colorsAttributeIndex = m_shader->getAttributeLocation("in_color");
			int texCoordsAttributeIndex = m_shader->getAttributeLocation("in_texcoord");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			int invModelLocation = m_shader->getUniformLocation("inv_model");
			glUniformMatrix3fv(invModelLocation, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m_model)))));

			int materialShininessLocation = m_shader->getUniformLocation("material.shininess");
			glUniform1f(materialShininessLocation, m_material.shininess);

			int textureLocation = m_shader->getUniformLocation("texSampler");

			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

			// Enable the attribute normals within the current VAO
			glEnableVertexAttribArray(normalsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_normals_id);
			glVertexAttribPointer(normalsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
			// Enable the attribute colors within the current VAO
	        /*glEnableVertexAttribArray(colorsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glVertexAttribPointer(colorsAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);*/
		 
			// Enable the attribute texcoords within the current VAO
			glEnableVertexAttribArray(texCoordsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glVertexAttribPointer(texCoordsAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_2D, m_textureID);
	        glUniform1i(textureLocation, 0);
			
			DrawArrays();

			glDisableVertexAttribArray(positionsAttributeIndex);
			glDisableVertexAttribArray(normalsAttributeIndex);
			//glDisableVertexAttribArray(colorsAttributeIndex);
			glDisableVertexAttribArray(texCoordsAttributeIndex);
		}

		void DrawArrays() {
			glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
		}

		void scaleTexCoords(const glm::vec2& extend_repeat) {
			for(unsigned int i = 0; i < m_tex_coords.size(); ++i) {
				m_tex_coords[i] *= extend_repeat;
			}

			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

	private:
		std::vector<glm::vec2>		m_tex_coords;
		std::vector<glm::vec3>		m_normals;

		GLuint						m_textureID;
		GLuint 						m_vertex_buffer_texcoords_id;
		GLuint 						m_vertex_buffer_normals_id;

		Material					m_material;
};

class Skybox {
	public:
	Skybox() {}
	~Skybox() {}

	static GLuint CreateBufferTexture() {
		GLuint textureID;
		glGenTextures(1, &textureID);
		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		std::vector<TexturePtr> images(6);
		images[0] = textures.get("right");
		images[1] = textures.get("left");
		images[2] = textures.get("top");
		images[3] = textures.get("bottom");
		images[4] = textures.get("back");
		images[5] = textures.get("front");

		for(unsigned int i = 0; i < images.size(); ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 3, images[i]->w, images[i]->h, 0, GL_BGR, GL_UNSIGNED_BYTE, images[i]->pixels);
		}

	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	    return textureID;
	}
};

template<>
class Renderable<Skybox> : public Drawable {
	public:
		Renderable(const ShaderProgramPtr shader) : Drawable(shader) {
			m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f)); 
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f)); // triangle 1 : end
		    m_positions.push_back(glm::vec3(0.5f, 0.5,-0.5f)); // triangle 2 : begin
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f)); // triangle 2 : end
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));

		    for(unsigned int i = 0; i < m_positions.size(); ++i) {
		    	m_positions[i] *= glm::vec3(SIZE_TILE);
		    }

						// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);
			// Positions
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_positions.size() * sizeof(glm::vec3),
						 m_positions.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_textureID = Skybox::CreateBufferTexture();
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
		}

		void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");

			int textureLocation = m_shader->getUniformLocation("skybox");

			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glDepthMask(GL_FALSE);
			glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
	        glUniform1i(textureLocation, 0);
			

			DrawArrays();
			glDepthMask(GL_TRUE);

			glDisableVertexAttribArray(positionsAttributeIndex);
		}

		void DrawArrays() {
			glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
		}

	private:
		std::vector<glm::vec2>		m_tex_coords;
		std::vector<glm::vec3>		m_normals;

		GLuint						m_textureID;
		GLuint 						m_vertex_buffer_texcoords_id;
};


class Billboard {
	public:
		Billboard() {}
		~Billboard() {}
};

template<>
class Renderable<Billboard> : public Drawable {
	public:
		struct Material {
			float shininess;
		};
		using Material = struct Material;

	public:
		Renderable(const ShaderProgramPtr shader,
				   const std::string& key_texture,
				   const glm::vec3& center,
				   const glm::vec2& size) : Drawable(shader),
				   							m_center(center),
				   							m_size(size) {
			Plane::initialize(m_positions,
							m_normals,
							m_colors,
							m_tex_coords);

			// Material
			m_material.shininess = 0.0f;

			// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);

			// Positions
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_positions.size() * sizeof(glm::vec3),
						 m_positions.data(),
						 GL_STATIC_DRAW);

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_textureID = CreateBufferTexture(key_texture);
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
		}

		void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int texCoordsAttributeIndex = m_shader->getAttributeLocation("in_texcoord");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			int invModelLocation = m_shader->getUniformLocation("inv_model");
			glUniformMatrix3fv(invModelLocation, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m_model)))));

			int materialShininessLocation = m_shader->getUniformLocation("material.shininess");
			glUniform1f(materialShininessLocation, m_material.shininess);

			int centerLocation = m_shader->getUniformLocation("center");
			glUniform3fv(centerLocation, 1, glm::value_ptr(m_center));

			int sizeLocation = m_shader->getUniformLocation("size");
			glUniform2fv(sizeLocation, 1, glm::value_ptr(m_size));

			int textureLocation = m_shader->getUniformLocation("texSampler");
			
			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		 
			// Enable the attribute texcoords within the current VAO
			glEnableVertexAttribArray(texCoordsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glVertexAttribPointer(texCoordsAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_2D, m_textureID);
	        glUniform1i(textureLocation, 0);
			
			DrawArrays();

			glDisableVertexAttribArray(positionsAttributeIndex);
			glDisableVertexAttribArray(texCoordsAttributeIndex);
		}

		void DrawArrays() {
			glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
		}

		void scaleTexCoords(const glm::vec2& extend_repeat) {
			for(unsigned int i = 0; i < m_tex_coords.size(); ++i) {
				m_tex_coords[i] *= extend_repeat;
			}

			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

	private:
		std::vector<glm::vec2>		m_tex_coords;
		std::vector<glm::vec3>		m_normals;

		glm::vec3					m_center;
		glm::vec2					m_size;

		GLuint						m_textureID;
		GLuint 						m_vertex_buffer_texcoords_id;

		Material					m_material;
};


template<>
class Renderable<MeshOBJ> : public Drawable {
	public:

		struct Material {
			float shininess;
		};
		using Material = struct Material;

		Renderable(const ShaderProgramPtr shader,
				   const std::string& key_obj,
				   const std::string& key_texture) : Drawable(shader) {

			MeshOBJPtr mesh = meshes.get(key_obj);
			if(mesh == nullptr) {
				std::cerr << "Error when loading the .OBJ mesh : " << key_obj << std::endl;
			}
			m_positions = mesh->getPositions();
			m_normals = mesh->getNormals();
			m_tex_coords = mesh->getTexcoords();

			m_material.shininess = 0.5f;

			// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_normals_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);
			// Positions
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_positions.size() * sizeof(glm::vec3),
						 m_positions.data(),
						 GL_STATIC_DRAW);

			// Normals
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_normals_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_normals.size() * sizeof(glm::vec3),
						 m_normals.data(),
						 GL_STATIC_DRAW);

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			m_textureID = CreateBufferTexture(key_texture);
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_normals_id);
			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
		}

		void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int normalsAttributeIndex = m_shader->getAttributeLocation("in_normal");
			int texCoordsAttributeIndex = m_shader->getAttributeLocation("in_texcoord");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			int invModelLocation = m_shader->getUniformLocation("inv_model");
			glUniformMatrix3fv(invModelLocation, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(m_model)))));

			int textureLocation = m_shader->getUniformLocation("texSampler");

			int materialShininessLocation = m_shader->getUniformLocation("material.shininess");
			glUniform1f(materialShininessLocation, m_material.shininess);

			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

			// Enable the attribute normals within the current VAO
			glEnableVertexAttribArray(normalsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_normals_id);
			glVertexAttribPointer(normalsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		 
			// Enable the attribute texcoords within the current VAO
			glEnableVertexAttribArray(texCoordsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glVertexAttribPointer(texCoordsAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_2D, m_textureID);
	        glUniform1i(textureLocation, 0);
			
			DrawArrays();

			glDisableVertexAttribArray(positionsAttributeIndex);
			glDisableVertexAttribArray(normalsAttributeIndex);
			glDisableVertexAttribArray(texCoordsAttributeIndex);
		}

		void DrawArrays() {
			glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
		}

		void scaleTexCoords(const glm::vec2& extend_repeat) {
			for(unsigned int i = 0; i < m_tex_coords.size(); ++i) {
				m_tex_coords[i] *= extend_repeat;
			}

			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

	private:
		std::vector<glm::vec2>		m_tex_coords;
		std::vector<glm::vec3>		m_normals;

		GLuint						m_textureID;
		GLuint 						m_vertex_buffer_texcoords_id;
		GLuint 						m_vertex_buffer_normals_id;

		Material					m_material;
};

template<>
class Renderable<Line> : public Drawable {
	public:
		Renderable(const ShaderProgramPtr shader, 
				   const glm::vec3& p1, const glm::vec3& p2, 
				   const glm::vec4& c1 = glm::vec4(1.f, 1.f, 1.f, 1.f),
				   const glm::vec4& c2 = glm::vec4(1.f, 1.f, 1.f, 1.f)) : Drawable(shader)  {
			setLine(p1, p2, c1, c2);

			// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_colors_id);

			// Positions
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_positions.size() * sizeof(glm::vec3),
						 m_positions.data(),
						 GL_STATIC_DRAW);

			// Colors
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_colors.size() * sizeof(glm::vec4),
						 m_colors.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void setLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& c1, const glm::vec4& c2) {
			m_positions.clear();
			m_positions.push_back(p1);
			m_positions.push_back(p2);

			m_colors.clear();
			m_colors.push_back(c1);
			m_colors.push_back(c2);
		}

		void DrawArrays() {
			glDrawArrays(GL_LINES, 0, m_positions.size());
		}
};

template<>
class Renderable<Box> : public Drawable {
	public:
		Renderable(const ShaderProgramPtr shader,
				   const glm::vec3& position,
				   const glm::vec3& size,
				   const glm::vec4& color) : Drawable(shader),
				   							 m_colors(36, color) {

			m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f)); 
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f)); // triangle 1 : end
		    m_positions.push_back(glm::vec3(0.5f, 0.5,-0.5f)); // triangle 2 : begin
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f)); // triangle 2 : end
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f,-0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(-0.5f, 0.5f, 0.5f));
		    m_positions.push_back(glm::vec3(0.5f,-0.5f, 0.5f));

	
			// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_colors_id);

			// Positions
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_positions.size() * sizeof(glm::vec3),
						 m_positions.data(),
						 GL_STATIC_DRAW);

			// Colors
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_colors.size() * sizeof(glm::vec4),
						 m_colors.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glm::vec3 unitWorld(SIZE_TILE, SIZE_TILE, SIZE_TILE);

			scaleLocalMatrix(size*unitWorld);
			translateHeritanceMatrix((position + size/2.f)*unitWorld);

			m_size = size;
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_colors_id);
		}

		void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int colorsAttributeIndex = m_shader->getAttributeLocation("in_color");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);

			// Enable the attribute normals within the current VAO
			glEnableVertexAttribArray(colorsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glVertexAttribPointer(colorsAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);

			DrawArrays();

			glDisableVertexAttribArray(positionsAttributeIndex);
			glDisableVertexAttribArray(colorsAttributeIndex);
		}

		void DrawArrays() {
			//glDisable(GL_DEPTH_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDrawArrays( GL_TRIANGLES, 0, m_positions.size());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//glEnable(GL_DEPTH_TEST);
		}

		/*void setPosition(const glm::vec3& position) {
			m_local = glm::mat4(1.0f);
			glm::vec3 unitWorld(SIZE_TILE, SIZE_TILE, SIZE_TILE);
			
			scaleModelMatrix(m_size*unitWorld);
			translateModelMatrix((position + m_size/2.f)*unitWorld);
		}*/

	private:
		std::vector<glm::vec4>		m_colors;

		glm::vec3					m_size;
		glm::vec3					m_center;
};


template<typename Primitive>
using RenderablePtr = std::shared_ptr<Renderable<Primitive>>;
