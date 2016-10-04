#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

#include "../include/Entity.h"

class EntityManager {
	public:
		EntityManager() {
		}

		~EntityManager() {
		}

		void add(const EntityPtr entity) {
			if(std::find(m_entitys.begin(), m_entitys.end(), entity) != m_entitys.end()) {
				std::cerr << "Entity has already been inserted" << std::endl;
				return;
			}
			m_entitys.push_back(entity);
		}

		void erase(const EntityPtr entity) {
			std::vector<EntityPtr>::iterator deleted_entity = std::find(m_entitys.begin(), m_entitys.end(), entity);
			if(deleted_entity == m_entitys.end()) {
				std::cerr << "Entity is not present" << std::endl;
				return;
			}

			m_entitys.erase(deleted_entity);
		}

		std::vector<EntityPtr>& getEntitys() {
			return m_entitys;
		}

		template<typename Component>
		static std::vector<EntityPtr> getEntitysByComponent(const std::vector<EntityPtr>& entitys) {
			std::vector<EntityPtr> entitys_by_component;
			for(auto& entity : entitys) {
				if(entity->getComponent<Component>() != nullptr) {
					entitys_by_component.push_back(entity);
				}
			}
			return entitys_by_component;
		}

	private:
		std::vector<EntityPtr>	m_entitys;
};
