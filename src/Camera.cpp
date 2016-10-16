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

#include "../include/renderable.hpp"

Camera::Camera(Mode mode) : m_orientation(0.f),
				   m_orientation_height(0.f),	
				   m_position(glm::vec3(100.f, 100.f, 100.f)),
				   //m_direction(glm::vec3(glm::cos(m_orientation), -0.2f, glm::sin(m_orientation))),
				   m_direction(1.f, 1.f, 0.f),
				   m_up(glm::vec3(0.f, 1.f, 0.f)),
				   m_mode(mode),
				   m_state(FORWARD),
				   m_near(0.1f),
				   m_far(500.f),
				   m_a(1920.f/1080.f),
				   m_fov(45.f),
				   m_screen(1920.f, 1080.f) {
	m_view = glm::lookAt
	(
	    m_position, // Camera is at ( 0, 0, -5), in World Space
	    glm::vec3(0.f), // And looks at the center
	    m_up  // Camera is upside-down
	);
	 
	m_projection = glm::perspective
	(
	    m_fov,              		// 45º field of view
	    m_a,  // 16:9 aspect ratio
	    m_near,               	// Only render what's 0.1 or more away from camera
	    m_far              	// Only render what's 100 or less away from camera
	);
	update_frustum_view();
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
			m_orientation_height = -3.14f/4.f;
			//movable->m_rotate = glm::rotate(glm::mat4(1.0f), -360.f*m_orientation/(2*3.14f), glm::vec3(0.f, 1.0f, 0.f));
			m_position = (movable->m_position - 6.0f*glm::vec3(glm::sin(3.14f/2.f - m_orientation_height)*glm::sin(3.14f/2.f - m_orientation), glm::cos(3.14f/2.f - m_orientation_height), glm::cos(3.14f/2.f - m_orientation)*glm::sin(3.14f/2.f - m_orientation_height)))*glm::vec3(SIZE_TILE);

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

	if(m_mode == PLAYER) {
		update_frustum_view();
	}
}

void Camera::update_frustum_view() {
	glm::vec3 right(m_view[0][0], m_view[1][0], m_view[2][0]);
	glm::vec3 up(m_view[0][1], m_view[1][1], m_view[2][1]);
	glm::vec3 forward(-m_view[0][2], -m_view[1][2], -m_view[2][2]);

	glm::vec3 np(forward*m_near);
	glm::vec3 fp(forward*m_far);

	float HnearHalf = m_near*glm::tan(m_fov/2.f);
	float WnearHalf = HnearHalf*m_a;

	glm::vec3 m_left(np - right*WnearHalf);
	glm::vec3 m_right(np + right*WnearHalf);
	glm::vec3 m_top(np + up*HnearHalf);
	glm::vec3 m_bottom(np - up*HnearHalf);
	m_left = glm::normalize(m_left);
	m_right = glm::normalize(m_right);
	m_top = glm::normalize(m_top);
	m_bottom = glm::normalize(m_bottom);

	m_frustum.m_planes[FrustumView::NEAR].n = -forward;
	m_frustum.m_planes[FrustumView::FAR].n = forward;
	m_frustum.m_planes[FrustumView::LEFT].n = glm::cross(up, m_left);
	m_frustum.m_planes[FrustumView::RIGHT].n = glm::cross(m_right, up);
	m_frustum.m_planes[FrustumView::TOP].n = glm::cross(right, m_top);
	m_frustum.m_planes[FrustumView::BOTTOM].n = glm::cross(m_bottom, right);

	//std::cout << glm::length(m_frustum.m_planes[FrustumView::NEAR].n) << std::endl;
	//std::cout << up.y << std::endl;
	//std::cout << m_frustum.m_planes[FrustumView::TOP].n.x << " " << m_frustum.m_planes[FrustumView::TOP].n.y << " " << m_frustum.m_planes[FrustumView::TOP].n.z << std::endl;

	compute_d_plane(m_position + np, m_frustum.m_planes[FrustumView::NEAR]);
	compute_d_plane(m_position + fp, m_frustum.m_planes[FrustumView::FAR]);
	compute_d_plane(m_position, m_frustum.m_planes[FrustumView::LEFT]);
	compute_d_plane(m_position, m_frustum.m_planes[FrustumView::RIGHT]);
	compute_d_plane(m_position, m_frustum.m_planes[FrustumView::TOP]);
	compute_d_plane(m_position, m_frustum.m_planes[FrustumView::BOTTOM]);
}

void Camera::compute_d_plane(const glm::vec3& p, Plane& plane) {
	plane.d = -glm::dot(plane.n, p);
}

const Camera::FrustumView& Camera::getFrustumView() const {
	return m_frustum;
}

