#pragma once

#include <vector>
#include <memory>
#include <array>

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
		const glm::vec3& getPosition() const;
		int getState() const;

		void setMode(Mode mode);

		void setPlayer(const PlayerPtr player);

		void update();

	private:
		void update_frustum_view();

		

	public:
		struct Plane {
			float 			d;
			glm::vec3		n;	
		};

		void compute_d_plane(const glm::vec3& p, Plane& plane);

		struct FrustumView {
			enum {
				NEAR, 
				FAR, 
				BOTTOM,
				TOP,
				LEFT,
				RIGHT,
			};

			std::array<Plane, 6>		m_planes;
		};

		const FrustumView& getFrustumView() const;
	private:

		glm::mat4	m_view;
		glm::mat4	m_projection;

		float 		m_orientation;
		float 		m_orientation_height;
		glm::vec3 	m_position;
		glm::vec3 	m_direction;
		glm::vec3 	m_up;

		Mode		m_mode;

		PlayerPtr	m_player;
		int 		m_state;
		bool 		m_walk;

		FrustumView m_frustum;
		float		m_near, m_far;
		float  		m_a;
		float 		m_fov;

		glm::vec2	m_screen;
};

using CameraPtr = std::shared_ptr<Camera>;

static CameraPtr camera = std::make_shared<Camera>(Camera::PLAYER);
