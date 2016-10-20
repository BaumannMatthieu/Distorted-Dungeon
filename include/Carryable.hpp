#pragma once

#include <vector>
#include <memory>
#include <set>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/Component.h"

#include "../include/MovableComponent.hpp"
#include "../include/CollisableComponent.hpp"
#include "../include/MotionSystem.hpp"

struct Carried : public Component {
	EntityPtr						m_entity;

	unsigned int					m_weight;
	unsigned int					m_gold;
	std::string						m_name;
	std::string						m_description;

	enum Type {WEAPON,
		 SHIELD,
		 ARMORY,
		 MISCI,
		 CONSO};

	Type 							m_type;

	std::pair<unsigned, unsigned>	m_damage;
	float							m_speed_attack;

	unsigned int					m_defense;
};

struct Container : public Component {
	std::vector<EntityPtr>		m_entitys;

	unsigned int 	m_max_weight;
	unsigned int 	m_max_armory;

	unsigned int    m_weight;
};

struct Equipped : public Component {
};

using EquippedPtr = std::shared_ptr<Equipped>;
using CarriedPtr = std::shared_ptr<Carried>;
using ContainerPtr = std::shared_ptr<Container>;

struct Effect;
using EffectPtr = std::shared_ptr<Effect>;

struct Effect {
	std::function<void (EffectPtr, EntityPtr)>			m_script;
};



struct Speed : public Effect {
	float							m_speed_factor;
};

struct Damage : public Effect {
	unsigned int 					m_damage;
};

using DamagePtr = std::shared_ptr<Damage>;

struct DoT : public Effect {
	float							m_damage_by_sec;
	float							m_duration_damage;
};

struct HoT : public Effect {
	float							m_heal_by_sec;
	float							m_duration_heal;
};

struct Move : public Effect {
	glm::vec3						m_position;
};

struct EffectCollided : public Component {
	std::set<EffectPtr>			m_effects;
};

using EffectCollidedPtr = std::shared_ptr<EffectCollided>;

struct Killable : public Component {
	float 		m_life;
	float 		m_defense;
};

using KillablePtr = std::shared_ptr<Killable>;

class Interaction {
	public:
		void run(std::vector<EntityPtr>& entitys) {
			SDL_PumpEvents();
			const Uint8 *keystate = SDL_GetKeyboardState(NULL);
			if(keystate[SDL_SCANCODE_Q]) {
				ContainerPtr container = m_player->getComponent<Container>();
				bool weapon_equiped = false;
				EntityPtr weapon = nullptr;

				for(auto& carried : container->m_entitys) {
					CarriedPtr object = carried->getComponent<Carried>();
					if(object->m_type == Carried::WEAPON) {
						EquippedPtr equipped = carried->getComponent<Equipped>();
						weapon_equiped = (equipped == nullptr);
						weapon = carried;
						break;
					}
				}

				if(weapon_equiped && weapon != nullptr) {
					MovablePtr movable = m_player->getComponent<Movable>();
					CollisablePtr<Cobble> player_collisable = m_player->getComponent<Collisable<Cobble>>();
					
					weapon->addComponent<Equipped>(std::make_shared<Equipped>());
				} else if(!weapon_equiped && weapon != nullptr) {
					weapon->deleteComponent<Equipped>();
				}
			}

			if(keystate[SDL_SCANCODE_0]) {
				ContainerPtr container = m_player->getComponent<Container>();
				EntityPtr weapon = nullptr;

				for(auto& carried : container->m_entitys) {
					CarriedPtr object = carried->getComponent<Carried>();
					EquippedPtr equipped = carried->getComponent<Equipped>();
					if(object->m_type == Carried::WEAPON/* && equipped != nullptr*/) {
						weapon = carried;
						break;
					}
				}

				if(weapon != nullptr && weapon->getComponent<Motion<Rotation>>() == nullptr) {
					MotionPtr<Rotation> motion = std::make_shared<Motion<Rotation>>();
					motion->m_alpha = glm::quat(1.f, 0.f, 0.f, 0.f);
					motion->m_beta = glm::quat(glm::cos(3.14f/4.f), 0.f, 0.f, glm::sin(3.14f/4.f));

					motion->m_time_start = SDL_GetTicks();
					motion->m_duration = 200;
					RenderableComponentPtr render = weapon->getComponent<RenderableComponent>();
					motion->m_heritance = render->m_renderable->getHeritanceMatrix();

					weapon->addComponent<Motion<Rotation>>(motion);
				}
			}
		}

		void setPlayer(const EntityPtr player) {
			m_player = player;
		}
	private:
		EntityPtr		m_player;
};