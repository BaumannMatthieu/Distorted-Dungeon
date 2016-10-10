#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/ShaderProgram.hpp"
#include "../include/renderable.hpp"



template<typename Type>
struct Particle : public Type {
	unsigned int 	m_duration;
	unsigned int	m_time_birth;

	glm::vec3		m_velocity;
	glm::vec3		m_position;
};

template<typename Type>
using ParticlePtr = std::shared_ptr<Particle<Type>>;

class Fire {
	public:
		static void update(ParticlePtr<Fire> particle, unsigned int dt) {
			particle->m_position += particle->m_velocity*glm::vec3(dt);
		}
};

template<typename Type>
class ParticleSystem {
	public:
		ParticleSystem(const RenderablePtr<ParticleSystem<Type>> render_system,
					const glm::vec3& position,
				   	const glm::vec2& size,
					unsigned int num_particles,
					GLuint textureID) : m_render_system(render_system),
												  m_position(position),
												  m_size(size),
												  m_textureID(textureID) {
			for(unsigned int i = 0; i < num_particles; ++i) {
				createNewParticle();
			}
		}

		void update(unsigned int dt) {
			std::vector<unsigned int> index_particles;
			
			for(unsigned int i = 0; i < m_particles.size(); ++i) {
				Type::update(m_particles[i], dt);
				if(SDL_GetTicks() - m_particles[i]->m_time_birth > m_particles[i]->m_duration) {
					index_particles.push_back(i);
				}
			}

			for(unsigned int i = 0; i < index_particles.size(); ++i) {
				unsigned int index = index_particles[i];

				deleteParticle(index);

				createNewParticle();
			}
		}

		void deleteParticle(unsigned int index) {
			m_render_particles.erase(m_render_particles.begin() + index);
			m_particles.erase(m_particles.begin() + index);
		}

		void createNewParticle() {
			ParticlePtr<Type> particle = std::make_shared<Particle<Type>>();

			particle->m_duration = 400 + 600*((float) std::rand() / RAND_MAX);
			particle->m_time_birth = SDL_GetTicks();
			particle->m_velocity = glm::vec3(-2.f*((float)std::rand() / RAND_MAX) + 1.f, -2.f*((float)std::rand() / RAND_MAX) + 1.f, -2.f*((float)std::rand() / RAND_MAX) + 1.f)*glm::vec3(0.0002f);
			particle->m_position = m_position;

			m_particles.push_back(particle);

			RenderablePtr<Particle<Type>> render_particle = std::make_shared<Renderable<Particle<Type>>>(m_render_system->getShaderProgram(),
																										 m_particles.back(),
																										 m_size,
																										 m_textureID);
			render_particle->setParent(m_render_system);

			m_render_particles.push_back(render_particle);
		}

		void draw() {
			glDepthMask(GL_FALSE);
			
			//glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			for(unsigned int i = 0; i < m_render_particles.size(); ++i) {
				m_render_particles[i]->draw();
			}
			glDisable(GL_BLEND);
			//glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
		}

	private:
		RenderablePtr<ParticleSystem<Type>> 			m_render_system;

		std::vector<RenderablePtr<Particle<Type>>>		m_render_particles;
		std::vector<ParticlePtr<Type>>					m_particles;

		glm::vec3 										m_position;
		glm::vec2										m_size;

		GLuint											m_textureID;
};

template<typename Type>
using ParticleSystemPtr = std::shared_ptr<ParticleSystem<Type>>;

template<typename Type>
class Renderable<ParticleSystem<Type>> : public Drawable, public std::enable_shared_from_this< Renderable< ParticleSystem<Type> > > {
	public:
		Renderable(const ShaderProgramPtr shader,
				   const std::string& key_texture) : Drawable(shader) {
			m_time_prec = SDL_GetTicks();
			m_textureID = CreateBufferTexture(key_texture);
		}

		void bindParticleSystem(const glm::vec3& position,
				   				const glm::vec2& size) {
			unsigned int num_particles = 25;

			translateHeritanceMatrix(position);
			m_particle_system = std::make_shared<ParticleSystem<Type>>(this->shared_from_this(),
														position,
														size,
														num_particles,
														m_textureID);
		}

		void draw() {
			m_particle_system->update(SDL_GetTicks() - m_time_prec);

			m_particle_system->draw();

			m_time_prec = SDL_GetTicks();
		}

		void DrawArrays() {
		}

	private:
		ParticleSystemPtr<Type> 						m_particle_system;
		unsigned int 									m_time_prec;

		GLuint											m_textureID;
};

template<typename Type>
class Renderable<Particle<Type>> : public Drawable {
	public:
		Renderable(const ShaderProgramPtr shader,
				   const ParticlePtr<Type> particle,
				   const glm::vec2& size,
				   GLuint textureID) : Drawable(shader),
				   							m_particle(particle),
				   							m_size(size),
				   							m_textureID(textureID) {
			Plane::initialize(m_positions,
							m_normals,
							m_colors,
							m_tex_coords);

			// Creation Vertex Buffer Objects
			glGenBuffers(1, &m_vertex_buffer_positions_id);
			glGenBuffers(1, &m_vertex_buffer_texcoords_id);
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

			// Texcoords
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glBufferData(GL_ARRAY_BUFFER,
						 m_tex_coords.size() * sizeof(glm::vec2),
						 m_tex_coords.data(),
						 GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);

			scaleLocalMatrix(glm::vec3(SIZE_TILE));
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_colors_id);
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
		}

		void draw() {
			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int texCoordsAttributeIndex = m_shader->getAttributeLocation("in_texcoord");
			int colorsAttributeIndex = m_shader->getAttributeLocation("in_color");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			int centerLocation = m_shader->getUniformLocation("particle.position");
			glUniform3fv(centerLocation, 1, glm::value_ptr(m_particle->m_position));

			int timeCreationLocation = m_shader->getUniformLocation("particle.time_birth");
			glUniform1i(timeCreationLocation, m_particle->m_time_birth);

			int durationLocation = m_shader->getUniformLocation("particle.duration");
			glUniform1i(durationLocation, m_particle->m_duration);

			int sizeLocation = m_shader->getUniformLocation("size");
			glUniform2fv(sizeLocation, 1, glm::value_ptr(m_size));

			int timeLocation = m_shader->getUniformLocation("time");
			glUniform1i(timeLocation, SDL_GetTicks());

			int textureLocation = m_shader->getUniformLocation("texSampler");
			
			// Enable the attribute positions within the current VAO
			glEnableVertexAttribArray(positionsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_positions_id);
			glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
		 
			// Enable the attribute texcoords within the current VAO
			glEnableVertexAttribArray(texCoordsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_texcoords_id);
			glVertexAttribPointer(texCoordsAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

			// Enable the attribute colors within the current VAO
			glEnableVertexAttribArray(colorsAttributeIndex);
			glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_colors_id);
			glVertexAttribPointer(colorsAttributeIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);

			glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_2D, m_textureID);
	        glUniform1i(textureLocation, 0);

			DrawArrays();

			glDisableVertexAttribArray(positionsAttributeIndex);
			glDisableVertexAttribArray(texCoordsAttributeIndex);
			glDisableVertexAttribArray(colorsAttributeIndex);
		}

		void DrawArrays() {
			glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
		}

	private:
		ParticlePtr<Type>			m_particle;

		std::vector<glm::vec2>		m_tex_coords;
		std::vector<glm::vec3>		m_normals;

		glm::vec2					m_size;

		GLuint						m_textureID;
		GLuint 						m_vertex_buffer_texcoords_id;
};

