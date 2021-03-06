#pragma once

#include <memory>
#include <vector>

#include "../include/System.h"
#include "../include/Entity.h"
#include "../include/Component.h"

#include "../include/Resource.hpp"
#include "../include/MovableComponent.hpp"
#include "../include/Carryable.hpp"
#include "../include/RenderableParticles.hpp"
#include "../include/Light.hpp"

#include <glm/gtx/vector_angle.hpp>

struct Caster : public Component {
	unsigned int				m_mana;
	unsigned int 				m_stamina;

	unsigned int				m_selected_technic;

	std::vector<EntityPtr>		m_entitys;
	std::vector<unsigned int>	m_last_time;
};

struct Triggerable : public Component {
	EntityPtr		m_thrower;

	unsigned int	m_mana_cost;
	unsigned int    m_stamina_cost;

	unsigned int	m_cooldown;
	unsigned int	m_duration;
};

using Caster = struct Caster;
using CasterPtr = std::shared_ptr<Caster>;

using TriggerablePtr = std::shared_ptr<Triggerable>;

class Magic {
	public:
		void run(std::vector<EntityPtr>& entitys) {	
			std::vector<EntityPtr> casters = EntityManager::getEntitysByComponent<Caster>(entitys);

			for(auto& entity : casters) {
	           	if(entity == m_player) {
	           		actionPlayer(entitys);
	           	} else {
					CasterPtr caster = entity->getComponent<Caster>();

					unsigned int technic_selected_id = caster->m_selected_technic;
					EntityPtr entity_triggerable = entity->getComponent<Caster>()->m_entitys[technic_selected_id];
					TriggerablePtr triggerable = entity_triggerable->getComponent<Triggerable>();

					if(triggerable->m_cooldown + caster->m_last_time[technic_selected_id] <= SDL_GetTicks()) {
						MovablePtr movable_enemy = m_player->getComponent<Movable>();
						MovablePtr movable = entity->getComponent<Movable>();

						// Launch the spell
						//createFireBowl(entitys, m_player, glm::vec3(camera->getDirection().x, 0.f, camera->getDirection().z));
						glm::vec3 direction = movable_enemy->m_position - movable->m_position;
						direction = glm::normalize(direction);

						createCircleFireBowl(entitys, entity, direction, 3.f, 10.f, glm::vec4(0.f, 1.f, 0.f, 1.f), 1);

						caster->m_last_time[technic_selected_id] = SDL_GetTicks();
					}
	           	}
	        }
		}

		void setPlayer(const EntityPtr player) {
			m_player = player;
		};

		void actionPlayer(std::vector<EntityPtr>& entitys) {
			CasterPtr caster = m_player->getComponent<Caster>();

			unsigned int technic_selected_id = caster->m_selected_technic;
			EntityPtr entity_triggerable = m_player->getComponent<Caster>()->m_entitys[technic_selected_id];
			TriggerablePtr triggerable = entity_triggerable->getComponent<Triggerable>();

			if(triggerable->m_cooldown + caster->m_last_time[technic_selected_id] <= SDL_GetTicks()) {
				SDL_PumpEvents();
				const Uint8 *keystate = SDL_GetKeyboardState(NULL);

				if(keystate[SDL_SCANCODE_SPACE]) {
					// Launch the spell
					//createFireBowl(entitys, m_player, glm::vec3(camera->getDirection().x, 0.f, camera->getDirection().z));
					createCircleFireBowl(entitys, m_player, glm::vec3(camera->getDirection().x, 0.f, camera->getDirection().z),
										 5.f,
										 20.f,
										 glm::vec4(1.f, 0.f, 0.f, 1.f),
										 1);

					caster->m_last_time[technic_selected_id] = SDL_GetTicks();
				}
			}
		};

		static void createFireBowl(std::vector<EntityPtr>& entitys,
							const EntityPtr& triggerer,
							const glm::vec3& direction,
							float speed,
							float damage,
							const glm::vec4& color) {
			EntityPtr firebowl = std::make_shared<Entity>();
									
			TriggerablePtr caract = std::make_shared<Triggerable>();
			caract->m_cooldown = 500;
			caract->m_duration = 0;
			caract->m_mana_cost = 20;
			caract->m_stamina_cost = 0;
			caract->m_thrower = triggerer;
			firebowl->addComponent<Triggerable>(caract);

			MovablePtr movable = std::make_shared<Movable>();
			movable->m_direction = direction;
			movable->m_position = triggerer->getComponent<Movable>()->m_position;
			movable->m_speed = speed;

			firebowl->addComponent<Movable>(movable);

			DamagePtr direct_damage = std::make_shared<Damage>();
			direct_damage->m_damage = damage;

			direct_damage->m_script = [](EffectPtr effet, EntityPtr opponent) {
				KillablePtr status = opponent->getComponent<Killable>();
				DamagePtr direct_damage = std::static_pointer_cast<Damage>(effet);
				float damage = (direct_damage->m_damage - status->m_defense);

				if(damage > 0.f) {
					status->m_life -= damage;
				}
			};

			EffectCollidedPtr effect = std::make_shared<EffectCollided>();
			effect->m_effects.insert(direct_damage);

			firebowl->addComponent<EffectCollided>(effect);

			glm::vec3 position_box(movable->m_position);
			glm::vec3 size_box(0.05f, 0.05f, 0.05f);
			glm::vec4 color_box(1.0f, 0.0f, 0.0f, 1.0f);

			CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();

			collisable->m_size = size_box;
			collisable->m_position = position_box;
			collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
																  collisable->m_position,
																  collisable->m_size,
																  color_box);
			firebowl->addComponent<Collisable<Cobble>>(collisable);

			RenderablePtr<ParticleSystem<Fire>> firebowl_particle_system = std::make_shared<Renderable<ParticleSystem<Fire>>>(shaders.get("fire"), "smoke", color);
			firebowl_particle_system->bindParticleSystem(movable->m_position, glm::vec2(0.1f));

			RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
			renderable_component->m_renderable = firebowl_particle_system;
			firebowl->addComponent<RenderableComponent>(renderable_component);

			LightPtr<Ponctual> light = std::make_shared<Light<Ponctual>>();
			light->m_position = glm::vec3(movable->m_position);
			
			light->m_ambiant = glm::vec3(color)/2.f;
			light->m_diffuse = glm::vec3(1.0f, 1.0f, 1.f);
			light->m_specular = glm::vec3(1.0f, 1.0f, 1.f);

			light->m_constant = 0.01f;
			light->m_linear = 0.02f;
			light->m_quadratic = 0.03f;
			firebowl->addComponent<Light<Ponctual>>(light);

			entitys.push_back(firebowl);
		}

		static void createCircleFireBowl(std::vector<EntityPtr>& entitys,
										 const EntityPtr& triggerer,
										 glm::vec3 direction_entity,
										 float speed,
										 float damage,
										 const glm::vec4& color,
										 unsigned int num_firebowl) {
			float dtheta = 2*3.14f/num_firebowl; 

			glm::vec2 u(1.0f, 0.f);
			glm::vec2 v(direction_entity.x, direction_entity.z);
			v = glm::normalize(v);
			float c = getCosAngle(u, v);
			float phi = 0.f;
			if(signAnglePositive(u, v)) {
				phi = glm::acos(c);
			} else {
				phi = -glm::acos(c);
			}

			for(unsigned int i = 0; i < num_firebowl; ++i) {
				glm::vec3 direction(glm::cos(dtheta*i + phi), 0.f, glm::sin(dtheta*i + phi));
				createFireBowl(entitys, triggerer, direction, speed, damage, color);
			}
		}

		static bool signAnglePositive(const glm::vec2& u, const glm::vec2& v) {
			float s = u.x*v.y - u.y*v.x;
			return (s > 0);
		}

		static float getCosAngle(const glm::vec2& u, const glm::vec2& v) {
			return glm::dot(u, v);
		}

	private:
		EntityPtr		m_player;
};
