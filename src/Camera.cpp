#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../include/CollidedComponent.hpp"

#include "../include/Camera.hpp"
#include "../include/Entity.h"

#include "../include/Dunjeon.hpp"

Camera::Camera(Mode mode) : m_orientation(0.f),
				   m_orientation_height(0.f),	
				   m_position(glm::vec3(0, 35.f, 0)),
				   //m_direction(glm::vec3(glm::cos(m_orientation), -0.2f, glm::sin(m_orientation))),
				   m_direction(0.f, -1.f, 0.f),
				   m_up(glm::vec3(0.f, 1.f, 0.f)),
				   m_screen(1920.f, 1080.f),
				   m_mode(mode),
				   m_state(FORWARD) {
	m_view = glm::lookAt
	(
	    m_position, // Camera is at ( 0, 0, -5), in World Space
	    m_position + m_direction, // And looks at the center
	    m_up  // Camera is upside-down
	);
	 
	m_projection = glm::perspective
	(
	    45.0f,              		// 45º field of view
	    m_screen.x / m_screen.y,  // 16:9 aspect ratio
	    0.1f,               	// Only render what's 0.1 or more away from camera
	    10000.0f              	// Only render what's 100 or less away from camera
	);
}

Camera::~Camera() {

}

const glm::mat4& Camera::getMatrixView() const {
	return m_view;
}

const glm::mat4& Camera::getMatrixProjection() const {
	return m_projection;
}

const glm::vec3& Camera::getDirection() const {
	return m_direction;
}

void Camera::setMode(Mode mode) {
	m_mode = mode;
}

void Camera::setPlayer(const PlayerPtr player) {
	m_player = player;

	if(m_mode == PLAYER && m_player != nullptr) {
		EntityPtr player = m_player->getEntity();

		/*PositionPtr pos_player = player->getComponent<Position>();
		if(pos_player != nullptr) {
			m_position = glm::vec3(pos_player->m_position.x*SIZE_TILE, 35.f, pos_player->m_position.y*SIZE_TILE);
		}*/
	}
}

int Camera::getState() const {
	return m_state;
}

void Camera::update() {
	m_walk = false;
	SDL_PumpEvents();
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	int mouse_position_x;
	int mouse_position_y;
	const Uint32 mouse_bitmask = SDL_GetMouseState(&(mouse_position_x), &(mouse_position_y));

	glm::vec2 center_to_mouse(glm::vec2(mouse_position_x, mouse_position_y) - m_screen*0.5f);

	int offset_x = 300;
	if(glm::abs(center_to_mouse.x) >= offset_x) {
		if(center_to_mouse.x > 0.f) {
			m_orientation += ((center_to_mouse.x - offset_x)/(m_screen.x*0.5f))*0.1f;
		} else {
			m_orientation += ((center_to_mouse.x + offset_x)/(m_screen.x*0.5f))*0.1f;
		}
	}

	int offset_y = 100;
	if(glm::abs(center_to_mouse.y) >= offset_y) {
		if(center_to_mouse.y > 0.f) {
			m_orientation_height -= ((center_to_mouse.y - offset_y)/(m_screen.y*0.5f))*0.03f;
		} else {
			m_orientation_height -= ((center_to_mouse.y + offset_y)/(m_screen.y*0.5f))*0.03f;
		}
	}
	
	//m_orientation_height -= (center_to_mouse.y/(m_screen.y*0.5f))*0.02f;

	if(m_orientation_height >= 3.14/2.f) {
		m_orientation_height = 3.14/2.f;
	} else if(m_orientation_height <= -3.14/2.f) {
		m_orientation_height = -3.14/2.f;
	}
	float delta_orientation = 0.0f;
	if(keystate[SDL_SCANCODE_A]) {
		delta_orientation = -0.05f;
	}

	if(keystate[SDL_SCANCODE_D]) {
		delta_orientation = 0.05f;
	}

	m_orientation += delta_orientation;

	//float speed = 0.0;
	m_direction = glm::vec3(glm::cos(m_orientation), glm::tan(m_orientation_height), glm::sin(m_orientation));	
	
	if(m_mode == GOD) {
		m_direction = glm::normalize(m_direction);
		
		if(keystate[SDL_SCANCODE_W]) {
			m_position += m_direction*2.f; 
		}

		if(keystate[SDL_SCANCODE_S]) {
			m_position -= m_direction*2.f; 
		}
	} else if(m_mode == PLAYER) {
		EntityPtr player = m_player->getEntity();
		
		MovablePtr movable = player->getComponent<Movable>();
		CollisablePtr<Cobble> collisable = player->getComponent<Collisable<Cobble>>();

		
		//collisable->m_box->rotateHeritanceMatrix(360.f*delta_orientation/(2*3.14f), glm::vec3(0.f, 1.0f, 0.f));

		//heritance_matrix = glm::scale(glm::mat4(1.0f), m_player->getSize());
		
		//PositionPtr pos_player = player->getComponent<Position>();

		//if(pos_player != nullptr) {
			/*if(m_state == FORWARD) {
				if(keystate[SDL_SCANCODE_S]) {
					//m_position -= m_direction*2.f*glm::vec3(1.0f, 0.f, 1.0f);
					movable->m_speed = -2.f; 

					m_state = BACK;
				}
			
				//pos_player->m_position = glm::vec2(m_position.x/SIZE_TILE, m_position.z/SIZE_TILE);
			} else if(m_state == BACK && player->getComponent<Movable>() == nullptr) {
				if(keystate[SDL_SCANCODE_W]) {
					m_position += m_direction*2.f*glm::vec3(1.0f, 0.f, 1.0f); 
					speed = 2.f;

					m_state = FORWARD;
				}

				//pos_player->m_position = glm::vec2(m_position.x/SIZE_TILE, m_position.z/SIZE_TILE);
			} else */
			//if(player->getComponent<Movable>() != nullptr) {
				movable->m_speed = 0.0f;
				if(keystate[SDL_SCANCODE_W]) {
					//m_position += m_direction*2.f*glm::vec3(1.0f, 0.f, 1.0f);
					movable->m_speed = 3.f;

					m_state = FORWARD;
					m_walk = true;
				}

				if(keystate[SDL_SCANCODE_S]) {
					//m_position -= m_direction*2.f*glm::vec3(1.0f, 0.f, 1.0f); 
					//speed = -2.f;
					movable->m_speed = -3.f;

					m_state = BACK;
					m_walk = true;
				}
				
				//pos_player->m_position = glm::vec2(m_position.x/SIZE_TILE, m_position.z/SIZE_TILE);
			//}
		//}
	
		// Update camera
		//EntityPtr player = m_player->getEntity();
		//PositionPtr pos_player = player->getComponent<Position>();

		//MovablePtr movable = player->getComponent<Movable>();
		/*if(movable != nullptr) {
			movable->m_direction = m_direction;
		} else {
			movable = std::make_shared<Movable>();
			movable->m_direction = glm::vec3(m_direction.x, 0.0f, m_direction.z);
			movable->m_position = glm::vec3(m_position.x/SIZE_TILE, 35.f/SIZE_TILE, m_position.z/SIZE_TILE);
			movable->m_speed = speed;
			player->addComponent<Movable>(movable);
			m_view = glm::lookAt
				(
				    m_position, // Camera is at ( 0, 0, -5), in World Space
				    m_position + m_direction, // And looks at the center
				    m_up  // Camera is upside-down
				);
			return;
		}*/
		//if(movable != nullptr) {
			//std::cout << "kdsfkldsklsdfkl" << std::endl;
			if(m_walk) {
				m_direction.y += glm::cos((float)SDL_GetTicks()*0.01f)*0.005f;
			}
			
			m_direction = glm::normalize(m_direction);

			movable->m_direction = glm::vec3(m_direction.x, 0.0f, m_direction.z);
			movable->m_quat = glm::quat(glm::cos(m_orientation/2.f), 0.f, glm::sin(-m_orientation/2.f), 0.f);
			//movable->m_rotate = glm::rotate(glm::mat4(1.0f), -360.f*m_orientation/(2*3.14f), glm::vec3(0.f, 1.0f, 0.f));
			m_position = movable->m_position*glm::vec3(SIZE_TILE);

			//collisable->m_box->setHeritanceMatrix(glm::translate(glm::mat4(1.0f), m_position)*glm::toMat4(movable->m_quat));

			//movable->m_quat = glm::quat(1.0f, 0.f, 0.f, 0.f);
			//movable->m_position = glm::vec3(m_position.x/SIZE_TILE, 35.f/SIZE_TILE, m_position.z/SIZE_TILE);
			//movable->m_speed = speed;


			/*glm::mat4 heritance_matrix(glm::mat4(1.0f));
			heritance_matrix = glm::rotate(glm::mat4(1.0f), -360.f*m_orientation/(2*3.14f), glm::vec3(0.f, 1.0f, 0.f));
			heritance_matrix = glm::translate(glm::mat4(1.0f), m_position)*heritance_matrix;
			collisable->m_box->setHeritanceMatrix(heritance_matrix);*/
		//}
	} 
	m_view = glm::lookAt
		(
		    m_position, // Camera is at ( 0, 0, -5), in World Space
		    m_position + m_direction, // And looks at the center
		    m_up  // Camera is upside-down
		);
}

