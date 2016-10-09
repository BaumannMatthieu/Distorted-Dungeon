#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../include/Player.hpp"
#include "../include/Position.hpp"
#include "../include/MovableComponent.hpp"

class Camera {
	public:
		enum Mode {
			GOD,
			PLAYER
		};

		enum {
			FORWARD,
			BACK
		};

		using Mode = enum Mode;

		Camera(Mode mode=GOD);
		~Camera();

		const glm::mat4& getMatrixView() const;
		const glm::mat4& getMatrixProjection() const;
		const glm::vec3& getDirection() const;
		int getState() const;

		void setMode(Mode mode);

		void setPlayer(const PlayerPtr player);

		void update();

	private:
		glm::mat4	m_view;
		glm::mat4	m_projection;

		float 		m_orientation;
		float 		m_orientation_height;
		glm::vec3 	m_position;
		glm::vec3 	m_direction;
		glm::vec3 	m_up;

		glm::vec2	m_screen;

		Mode		m_mode;

		PlayerPtr	m_player;
		int 		m_state;
		bool 		m_walk;
};

using CameraPtr = std::shared_ptr<Camera>;

static CameraPtr camera = std::make_shared<Camera>(Camera::PLAYER);
