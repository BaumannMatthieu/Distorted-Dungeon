#pragma once

#include <memory>

#include "../include/Dunjeon.hpp"
#include "../include/Entity.h"
#include "../include/EntityManager.h"

class Player {
	public:
		Player(const DungeonPtr dungeon, EntityManager& entity_manager);
		~Player();

		EntityPtr getEntity() const;

		const glm::vec3 getSize() const;
		const glm::vec3 getPosition() const;

	private:
		EntityPtr		m_entity;
};

using PlayerPtr = std::shared_ptr<Player>;