#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/EntityManager.h"

#include "../include/Player.hpp"
#include "../include/Dunjeon.hpp"

#include "../include/RenderSystem.hpp"
#include "../include/ColliderSystem.hpp"
#include "../include/CollidedSystem.hpp"
#include "../include/MagicSystem.hpp"
#include "../include/PhysicSystem.hpp"
#include "../include/MotionSystem.hpp"
#include "../include/AI.hpp"
#include "../include/Carryable.hpp"
#include "../include/HUD.hpp"

class Scene {
	public:
		Scene();
		~Scene();

		bool run(bool pause);

	private:
		bool isPlayerDead() const;

	private:
		EntityManager				m_entitys;

		PlayerPtr					m_player;
		DungeonPtr					m_dungeon;
		RenderablePtr<Skybox>		m_skybox;

		System<Render>				m_renderer;
		System<Collider>			m_collider;
		System<Collided>			m_collided;
		System<Magic>				m_magic;
		System<PhysicManager>		m_physic;
		System<MotionManager>		m_motion;
		System<Interaction>			m_interaction;
		System<AISystem>			m_ai;
		HUDPtr						m_gui;

		GLuint 						m_vertex_array_id;
};

using ScenePtr = std::shared_ptr<Scene>;