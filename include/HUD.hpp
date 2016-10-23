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
#include "../include/RenderableText.hpp"

struct UIElement {
	glm::vec2			m_position_screen;
};

template<>
class Renderable<UIElement> : public Drawable,
							  public UIElement {
	public:
		static const glm::vec2 pixelToScreenCoordinates(const glm::vec2& position) {
			glm::vec2 position_screen = position;
			
			position_screen *= glm::vec2(2.f/WINDOW_WIDTH, 2.f/WINDOW_HEIGHT);
			position_screen -= glm::vec2(1.f);
			position_screen.y *= -1.f;

			return position_screen;
		}

		Renderable(const ShaderProgramPtr shader,
				   const std::string& key_texture,
				   const glm::vec2& position_screen,
				   const glm::vec2& size) : Drawable(shader),
				   							m_size(size) {
			Plane::initialize(m_positions,
							m_normals,
							m_colors,
							m_tex_coords);

			m_position_screen = pixelToScreenCoordinates(position_screen);
			m_size = size*glm::vec2(2.f/WINDOW_WIDTH, 2.f/WINDOW_HEIGHT);

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

			scaleLocalMatrix(glm::vec3(SIZE_TILE));
		}

		virtual ~Renderable() {
			glDeleteBuffers(1, &m_vertex_buffer_positions_id);
			glDeleteBuffers(1, &m_vertex_buffer_texcoords_id);
		}

		void draw(bool triggered) {
			m_shader->bind();

			int viewLocation = m_shader->getUniformLocation("view");
			int projectionLocation = m_shader->getUniformLocation("projection");
			glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera->getMatrixView()));
			glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera->getMatrixProjection()));

			int positionsAttributeIndex = m_shader->getAttributeLocation("in_position");
			int texCoordsAttributeIndex = m_shader->getAttributeLocation("in_texcoord");

			int modelLocation = m_shader->getUniformLocation("model");
			computeModelMatrix();
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(m_model));

			int triggeredLocation = m_shader->getUniformLocation("triggered");
			glUniform1i(triggeredLocation, triggered);

			int positionScreenLocation = m_shader->getUniformLocation("position_screen");
			glUniform2fv(positionScreenLocation, 1, glm::value_ptr(m_position_screen));

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

			m_shader->unbind();
		}

		void DrawArrays() {
			glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
		}

	private:
		std::vector<glm::vec2>		m_tex_coords;
		std::vector<glm::vec3>		m_normals;

		glm::vec3					m_center;
		glm::vec2					m_size;

		GLuint						m_textureID;
		GLuint 						m_vertex_buffer_texcoords_id;
};

class Button {
	public:
		Button(const glm::vec2& center,
			   const std::string& text,
			   FT_Face face) : m_cliked(false),
			   				   m_enforced(false),
			   				   m_has_clicked(false) {
			FT_Set_Pixel_Sizes(face, 0, 32);
			m_text = std::make_shared<Renderable<UIText>>(shaders.get("font"), text, face, center);

			glm::vec2 offsetText(30.f, 20.f);
			m_size = offsetText + m_text->getSizeText();
			m_frame = std::make_shared<Renderable<UIElement>>(shaders.get("UI"), "wall", center, m_size);

			m_position = center - m_size/2.f;
		}

		void draw() {
			m_frame->draw(m_has_clicked && m_in_button);
			m_text->draw();
		}

		bool clicked() {
			int mouse_position_x;
			int mouse_position_y;
			const Uint32 mouse_bitmask = SDL_GetMouseState(&(mouse_position_x), &(mouse_position_y));

			m_cliked = false;
			m_in_button = false;

			if(mouse_position_x >= m_position.x && mouse_position_x <= m_position.x + m_size.x &&
				   mouse_position_y >= m_position.y && mouse_position_y <= m_position.y + m_size.y) {
				m_in_button = true;
			}

			if(mouse_bitmask & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				if(m_in_button && !m_enforced) {
					//m_cliked = true;
					m_has_clicked = true;
				}
				m_enforced = true;
			} else {
				if(m_enforced && m_has_clicked && m_in_button) {
					m_cliked = true;
				}

				m_enforced = false;
				m_has_clicked = false;
			}

			return m_cliked;
		}

	private:
		
		glm::vec2		m_position;
		glm::vec2		m_size;

		RenderablePtr<UIText>		m_text;
		RenderablePtr<UIElement>	m_frame;

		bool						m_cliked;
		bool						m_enforced;
		bool						m_has_clicked;
		bool						m_in_button;
};

using ButtonPtr = std::shared_ptr<Button>;

class HUD {
	public:
		HUD(const EntityPtr player) : m_player(player) {
			glm::vec2 size_ui = glm::vec2(100.f);

			glm::vec2 position_init(size_ui.x/2.f, WINDOW_HEIGHT - size_ui.y/2.f);
			glm::vec2 position_screen = position_init;

			//glm::vec2 size_ui_projected = size_ui*glm::vec2(2.f/WINDOW_WIDTH, 2.f/WINDOW_HEIGHT);

			if(FT_Init_FreeType(&m_ft)) {
				std::cerr << "Could not init freetype library" << std::endl;
			}

			if(FT_New_Face(m_ft, "../data/font/agalar.ttf", 0, &m_face)) {
			  	std::cerr << "Could not open font" << std::endl;
			}

			m_button = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), "Back To Game !", m_face);
			/*for(int i = 0; i < 5; ++i) {
				m_lifebar_elements.push_back(std::make_shared<Renderable<UIElement>>(shaders.get("UI"), "ward-texture", position_screen, size_ui));
				
				position_screen += glm::vec2(size_ui.x + 10.f, 0.f);
			}*/
		}

		void draw() {
			glDepthMask(GL_FALSE);
			for(auto& lifebar_element : m_lifebar_elements) {
				lifebar_element->draw(false);
			}
			//m_button->draw();
			glDepthMask(GL_TRUE);

			if(m_button->clicked()) {
				std::cout << "clicked ! " << std::endl;
			}
		}

		void update() {
			//KillablePtr life = m_player->getComponent<Killable>();
		}


	private:

		FT_Library 			m_ft;
		FT_Face 			m_face;

		EntityPtr 			m_player;
		ButtonPtr	  		m_button;

		std::vector<RenderablePtr<UIElement>>		m_lifebar_elements;
};

using HUDPtr = std::shared_ptr<HUD>;


