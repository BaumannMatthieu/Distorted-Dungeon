#pragma once

#include <vector>
#include <memory>

#include "../include/System.h"
#include "../include/Entity.h"

#include "../include/Component.h"

#include "../include/MovableComponent.hpp"
#include "../include/CollisableComponent.hpp"

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

template<typename Type>
struct Effect;

template<typename Type>
using EffectPtr = std::shared_ptr<Effect<Type>>;

template<typename Type>
struct Effect : public Component, public Type {
	std::function<void (const EntityPtr, EffectPtr<Type>)>		m_script;
};

struct Speed {
	float							m_speed_factor;
};

struct DoT {
	float							m_damage_by_sec;
	float							m_duration_damage;
};

struct HoT {
	float							m_heal_by_sec;
	float							m_duration_heal;
};

struct Move {
	glm::vec3						m_position;
};

template<typename Type>
using EffectPtr = std::shared_ptr<Effect<Type>>;

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

					CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
					collisable->m_position = glm::vec3(-0.1f, -0.3f, 0.1f);
					collisable->m_size = glm::vec3(0.3f, 0.1f, 0.1f);
					collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																		  collisable->m_position,
																		  collisable->m_size,
																		  glm::vec4(1.f, 1.f, 0.f, 1.f));
					CollisablePtr<Cobble> player_collisable = m_player->getComponent<Collisable<Cobble>>();
					collisable->m_box->setParent(player_collisable->m_box);

					RenderablePtr<MeshOBJ> axe_mesh = std::make_shared<Renderable<MeshOBJ>>(shaders.get("textured"), "axe", "layout axe");
					RenderableComponentPtr render = std::make_shared<RenderableComponent>();
					render->m_renderable = axe_mesh;
					axe_mesh->setParent(player_collisable->m_box);
					glm::vec3 unitWorld(SIZE_TILE, SIZE_TILE, SIZE_TILE);
					axe_mesh->scaleLocalMatrix(glm::vec3(2.0f, 2.0f, 2.0f));
					//axe_mesh->rotateLocalMatrix(90.f, glm::vec3(0.0f, 0.0f, -1.0f));
					axe_mesh->rotateLocalMatrix(180.f, glm::vec3(0.0f, 1.0f, 0.0f));
					axe_mesh->translateLocalMatrix(glm::vec3(0.3f, 0.f, 0.15f)*unitWorld);
					//axe_mesh->scaleTexCoords(glm::vec2(2.0f, 2.0f));
					weapon->addComponent<RenderableComponent>(render);

					weapon->addComponent<Collisable<Cobble>>(collisable);
					weapon->addComponent<Equipped>(std::make_shared<Equipped>());
				} else if(!weapon_equiped && weapon != nullptr) {
					weapon->deleteComponent<Collisable<Cobble>>();
					weapon->deleteComponent<Equipped>();
					weapon->deleteComponent<RenderableComponent>();
				}
			}
		}

		void setPlayer(const EntityPtr player) {
			m_player = player;
		}
	private:
		EntityPtr		m_player;
};