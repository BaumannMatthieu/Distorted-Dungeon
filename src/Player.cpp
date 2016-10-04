#include <vector>

#include "../include/Player.hpp"
#include "../include/Position.hpp"
#include "../include/MovableComponent.hpp"
#include "../include/Physicable.hpp"
#include "../include/MagicSystem.hpp"
#include "../include/Carryable.hpp"
#include "../include/Camera.hpp"

Player::Player(const DungeonPtr dungeon, EntityManager& entity_manager) {
	m_entity = std::make_shared<Entity>();

	// Determining beginning position
	std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>> rooms = dungeon->getRooms();
	unsigned int index = std::rand() % rooms.size();

	//PositionPtr position = std::make_shared<Position>();
	//position->m_position = glm::vec2(rooms[index].getCenter().x, rooms[index].getCenter().z);
	//m_entity->addComponent<Position>(position);
	
	MovablePtr movable = std::make_shared<Movable>();
	movable->m_direction = camera->getDirection();
	movable->m_position = glm::vec3(rooms[index].getCenter().x, 10.73f, rooms[index].getCenter().z);
	movable->m_speed = 0.f;
	movable->m_quat = glm::quat(1.f, 0.f, 0.f, 0.f);
	m_entity->addComponent<Movable>(movable);

	CasterPtr caster = std::make_shared<Caster>();
	caster->m_selected_spell = 0;
	SpellPtr firebowl = std::make_shared<Spell>();
	firebowl->m_cost = 20;
	firebowl->m_cooldown = 1000;
	firebowl->m_duration = 0;
	caster->m_spells.push_back(firebowl);
	caster->m_last_time.push_back(SDL_GetTicks());
	m_entity->addComponent<Caster>(caster);

	CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
	collisable->m_position = getPosition();
	//collisable->m_position_start = collisable->m_position;
	collisable->m_size = getSize();
	collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
														  collisable->m_position,
														  collisable->m_size,
														  glm::vec4(1.f, 1.f, 1.f, 1.f));
	m_entity->addComponent<Collisable<Cobble>>(collisable);

	PhysicPtr physic = std::make_shared<Physic>();
	physic->m_gravity = 9.81f;
	physic->m_mass = 15.f;
	physic->m_jump = false;
	m_entity->addComponent<Physic>(physic);

	// weapon
	EntityPtr weapon = std::make_shared<Entity>();
	CarriedPtr carried = std::make_shared<Carried>();
	carried->m_entity = m_entity;

	carried->m_weight = 20;
	carried->m_gold = 100;
	carried->m_name = "Stupid Axe for Goblin' slaves";
	carried->m_description = "Do not hurt a lot. Just 5-10 damage point every 1.5 sec to his enemy";

	carried->m_type = Carried::WEAPON;

	carried->m_damage = std::make_pair(5, 10);
	carried->m_speed_attack = 1.5f;

	carried->m_defense = 0;
	weapon->addComponent<Carried>(carried);

	ContainerPtr container = std::make_shared<Container>();
	container->m_max_weight = 100;
	container->m_max_armory = 5;
	container->m_entitys.push_back(weapon);
	m_entity->addComponent<Container>(container);

	entity_manager.add(weapon);
	entity_manager.add(m_entity);
}

const glm::vec3 Player::getSize() const {
	return glm::vec3(0.5f, 1.0f, 0.5f);
}

const glm::vec3 Player::getPosition() const {
	MovablePtr movable = m_entity->getComponent<Movable>();
	return glm::vec3(movable->m_position.x - 0.25f, 10.0f, movable->m_position.z - 0.25f);
}

Player::~Player() {

}

EntityPtr Player::getEntity() const {
	return m_entity;
}