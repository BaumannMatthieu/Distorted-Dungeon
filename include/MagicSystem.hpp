#pragma once

#include <memory>
#include <vector>

#include "../include/System.h"
#include "../include/Entity.h"
#include "../include/Component.h"

#include "../include/Resource.hpp"
#include "../include/MovableComponent.hpp"

struct Damage : public Component {
	unsigned int 	m_damage;
};

using Damage = struct Damage;
using DamagePtr = std::shared_ptr<Damage>;

struct Spell : public Component {
	unsigned int	m_cost;

	unsigned int	m_cooldown;
	unsigned int	m_duration;
};

using Spell = struct Spell;
using SpellPtr = std::shared_ptr<Spell>;

struct Dot : public Component {
	unsigned int	m_interval_time;
};

using Dot = struct Dot;
using DotPtr = std::shared_ptr<Dot>;

struct Caster : public Component {
	unsigned int				m_mana;

	unsigned int				m_selected_spell;

	std::vector<SpellPtr>		m_spells;
	std::vector<unsigned int>	m_last_time;
};

using Caster = struct Caster;
using CasterPtr = std::shared_ptr<Caster>;

class Magic {
	public:
		void run(std::vector<EntityPtr>& entitys) {	
			std::vector<EntityPtr> casters = EntityManager::getEntitysByComponent<Caster>(entitys);

			for(auto& entity : casters) {
	           	if(entity == m_player) {
	           		actionPlayer(entitys);
	           	}
	        }
		}

		void setPlayer(const PlayerPtr player) {
			m_player = player->getEntity();
		};

		void actionPlayer(std::vector<EntityPtr>& entitys) {
			CasterPtr caster = m_player->getComponent<Caster>();

			unsigned int spell_selected_id = caster->m_selected_spell;
			SpellPtr spell_selected = m_player->getComponent<Caster>()->m_spells[spell_selected_id];

			if(spell_selected->m_cooldown + caster->m_last_time[spell_selected_id] <= SDL_GetTicks()) {
				SDL_PumpEvents();
				const Uint8 *keystate = SDL_GetKeyboardState(NULL);

				if(keystate[SDL_SCANCODE_SPACE]) {
					std::cout << "SPELL ! " << std::endl;
					// Launch the spell
					EntityPtr spell = std::make_shared<Entity>();
									
					SpellPtr caract = std::make_shared<Spell>();
					caract->m_cooldown = 1000;
					caract->m_duration = 0;
					caract->m_cost = 20;
					spell->addComponent<Spell>(caract);

					DamagePtr damage = std::make_shared<Damage>();
					damage->m_damage = 20;
					spell->addComponent<Damage>(damage);

					/*PositionPtr position = std::make_shared<Position>();
					position->m_position = m_player->getComponent<Position>()->m_position;
					spell->addComponent<Position>(position);*/

					MovablePtr movable = std::make_shared<Movable>();
					movable->m_direction = camera->getDirection();
					movable->m_position = m_player->getComponent<Movable>()->m_position;
					movable->m_speed = 0.01f;
					spell->addComponent<Movable>(movable);

					glm::vec3 position_box(movable->m_position);
					glm::vec3 size_box(0.1f, 0.1f, 0.1f);
					glm::vec4 color_box(1.0f, 0.0f, 0.0f, 1.0f);

					RenderablePtr<Box> box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																					  position_box,
																					  size_box,
																					  color_box);
					RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
					renderable_component->m_renderable = box;
					spell->addComponent<RenderableComponent>(renderable_component);

					entitys.push_back(spell);

					caster->m_last_time[spell_selected_id] = SDL_GetTicks();
				}
			}
		};

	private:
		EntityPtr		m_player;
};