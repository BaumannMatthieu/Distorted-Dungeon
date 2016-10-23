#pragma once

#include <vector>
#include <memory>
#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../include/System.h"
#include "../include/Entity.h"
#include "../include/Component.h"
#include "../include/Config.hpp"

#include "../include/renderable.hpp"
#include "../include/Camera.hpp"

struct UIText {
	std::string			m_text;
	glm::vec2			m_position_screen;
};

template<>
class Renderable<UIText> : public Drawable,
						   public UIText {
	public:
		static const glm::vec2 pixelToScreenCoordinates(const glm::vec2& position) {
			glm::vec2 position_screen = position;
			
			position_screen *= glm::vec2(2.f/WINDOW_WIDTH, 2.f/WINDOW_HEIGHT);
			position_screen -= glm::vec2(1.f);
			position_screen.y *= -1.f;

			return position_screen;
		}



		Renderable(const ShaderProgramPtr shader,
				   const std::string& text,
				   FT_Face face,
				   const glm::vec2& position_screen) : Drawable(shader) {
			m_text = text;
			m_position_screen = position_screen;

			m_texturesID.resize(m_text.size());
			m_vertex_buffer_pos_id.resize(m_text.size());
			m_vertex_buffer_tx_id.resize(m_text.size());

			glm::vec2 size_text(0.f);
			float max_height_glyph = 0.f;

		  	for(unsigned int i = 0; i < m_text.size(); ++i) {
			    if(FT_Load_Char(face, m_text[i], FT_LOAD_RENDER)) {
			        continue;
				}

				glActiveTexture(GL_TEXTURE0);
				glGenTextures(1, &m_texturesID[i]);
				glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				FT_GlyphSlot g = face->glyph;

			    glTexImage2D(
			      GL_TEXTURE_2D,
			      0,
			      GL_RED,
			      g->bitmap.width,
			      g->bitmap.rows,
			      0,
			      GL_RED,
			      GL_UNSIGNED_BYTE,
			      g->bitmap.buffer
			    );
			 
			    glm::vec2 position_glyph = m_position_screen + glm::vec2(g->bitmap_left, -g->bitmap_top);

			    float w = g->bitmap.width;
			    float h = g->bitmap.rows;

			    if(h > max_height_glyph) {
			    	max_height_glyph = h;
			    }
			 
			 	std::vector<glm::vec3> m_pos_letter;
			 	std::vector<glm::vec2> m_tx_letter;

			 	m_pos_letter.push_back(glm::vec3(pixelToScreenCoordinates(position_glyph), 0.f));
			 	m_pos_letter.push_back(glm::vec3(pixelToScreenCoordinates(position_glyph + glm::vec2(w, 0.f)), 0.f));
			 	m_pos_letter.push_back(glm::vec3(pixelToScreenCoordinates(position_glyph + glm::vec2(w, h)), 0.f));
			 	m_pos_letter.push_back(glm::vec3(pixelToScreenCoordinates(position_glyph + glm::vec2(0.f, h)), 0.f));

			 	m_tx_letter.push_back(glm::vec2(0.f, 0.f));
			 	m_tx_letter.push_back(glm::vec2(1.f, 0.f));
			 	m_tx_letter.push_back(glm::vec2(1.f, 1.f));
			 	m_tx_letter.push_back(glm::vec2(0.f, 1.f));

			 	m_pos.push_back(m_pos_letter);
			 	m_tex_coords.push_back(m_tx_letter);

			    m_position_screen += glm::vec2(g->advance.x/64.f, g->advance.y/64.f);
			    size_text += glm::vec2(g->advance.x/64.f, 0.f);

			    glBindTexture(GL_TEXTURE_2D, 0);
		  	}
		  	size_text += glm::vec2(0.f, max_height_glyph);

		  	m_size_text = size_text;

		  	size_text /= glm::vec2(2.f);
		  	size_text *= glm::vec2(2.f/WINDOW_WIDTH, 2.f/WINDOW_HEIGHT);

		  	for(unsigned int i = 0; i < m_text.size(); ++i) {
		  		for(unsigned int j = 0; j < 4; ++j) {
		  			m_pos[i][j].x -= size_text.x;
		  			m_pos[i][j].y -= size_text.y;
		  		}

		  		// Creation Vertex Buffer Objects
				glGenBuffers(1, &m_vertex_buffer_pos_id[i]);
				glGenBuffers(1, &m_vertex_buffer_tx_id[i]);

				// Positions
				glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_pos_id[i]);
				glBufferData(GL_ARRAY_BUFFER,
							 m_pos[i].size() * sizeof(glm::vec3),
							 m_pos[i].data(),
							 GL_STATIC_DRAW);

				// Texcoords
				glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_tx_id[i]);
				glBufferData(GL_ARRAY_BUFFER,
							 m_tex_coords[i].size() * sizeof(glm::vec2),
							 m_tex_coords[i].data(),
							 GL_STATIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
		  	}
		}

		virtual ~Renderable() {
		  	for(unsigned int i = 0; i < m_text.size(); ++i) {
				glDeleteBuffers(1, &m_vertex_buffer_pos_id[i]);
				glDeleteBuffers(1, &m_vertex_buffer_tx_id[i]);
			}
		}

		const glm::vec2& getSizeText() const {
			return m_size_text;
		}

		void draw() {
			m_shader->bind();

			for(unsigned int i = 0; i < m_text.size(); ++i) {
				int viewLocation = m_shader->getUniformLocation("view");
				int projectionLocation = m_shader->getUniformLocation("projection");
				glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->getMatrixView()));
				glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->getMatrixProjection()));

				int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
				int texCoordsAttributeIndex = m_shader->getAttributeLocation("in_texcoord");

				int modelLocation = m_shader->getUniformLocation("model");
				computeModelMatrix();
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

				int textureLocation = m_shader->getUniformLocation("texSampler");
				
				// Enable the attribute positions within the current VAO
				glEnableVertexAttribArray(positionsAttributeIndex);
				glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_pos_id[i]);
				glVertexAttribPointer(positionsAttributeIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
			 
				// Enable the attribute texcoords within the current VAO
				glEnableVertexAttribArray(texCoordsAttributeIndex);
				glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_tx_id[i]);
				glVertexAttribPointer(texCoordsAttributeIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);

				glActiveTexture(GL_TEXTURE0);
		        glBindTexture(GL_TEXTURE_2D, m_texturesID[i]);
		        glUniform1i(textureLocation, 0);
				
				DrawArrays();

				glDisableVertexAttribArray(positionsAttributeIndex);
				glDisableVertexAttribArray(texCoordsAttributeIndex);
			}

			m_shader->unbind();
		}

		void DrawArrays() {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glDrawArrays(GL_QUADS, 0, 4);
			glDisable(GL_BLEND);
		}

	private:
		std::vector<std::vector<glm::vec2>>		m_tex_coords;
		std::vector<std::vector<glm::vec3>>		m_pos;

		glm::vec2					m_size_text;

		std::vector<GLuint>			m_texturesID;
		
		std::vector<GLuint>			m_vertex_buffer_tx_id;
		std::vector<GLuint>			m_vertex_buffer_pos_id;
};
