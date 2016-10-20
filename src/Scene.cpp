#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/Scene.hpp"
#include "../include/Camera.hpp"

#include "../include/MeshOBJRenderable.hpp"

#include "../include/RenderableComponent.hpp"
#include "../include/RenderableParticles.hpp"
#include "../include/MovableComponent.hpp"

#include "../include/RenderSystem.hpp"
#include "../include/ColliderSystem.hpp"
#include "../include/MotionSystem.hpp"

#include "../include/Dunjeon.hpp"
#include "../include/Light.hpp"
#include "../include/Player.hpp"

static std::default_random_engine generator;

void createAICharacter(const DungeonPtr dungeon, EntityManager& entity_manager) {
	EntityPtr entity = std::make_shared<Entity>();
	
	std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>> rooms = dungeon->getRooms();
	unsigned int index = std::rand() % rooms.size();
	// Determining beginning position
	MovablePtr movable = std::make_shared<Movable>();
	movable->m_direction = camera->getDirection();
	movable->m_position = glm::vec3(rooms[index].getCenter().x, 0.f, rooms[index].getCenter().z);
	movable->m_speed = 0.f;
	movable->m_quat = glm::quat(1.f, 0.f, 0.f, 0.f);
	
	entity->addComponent<Movable>(movable);

	//PositionPtr position = std::make_shared<Position>();
	//position->m_position = glm::vec2(rooms[index].getCenter().x, rooms[index].getCenter().z);
	//m_entity->addComponent<Position>(position);
	RenderablePtr<MeshOBJ> goblin_mesh = std::make_shared<Renderable<MeshOBJ>>(shaders.get("textured"), "ward", "ward-texture");
	RenderableComponentPtr render = std::make_shared<RenderableComponent>();
	render->m_renderable = goblin_mesh;
	//glm::vec3 unitWorld(SIZE_TILE, SIZE_TILE, SIZE_TILE);
	goblin_mesh->scaleLocalMatrix(glm::vec3(0.1f));
	//goblin_mesh->rotateLocalMatrix(90.f, glm::vec3(0.0f, 1.0f, 0.0f));
	render->m_renderable->translateHeritanceMatrix(movable->m_position);

	entity->addComponent<RenderableComponent>(render);
	movable->m_heritance = goblin_mesh->getHeritanceMatrix();

	CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();

	//collisable->m_position_start = collisable->m_position;
	collisable->m_size = getSizePositions(goblin_mesh, 0.1f);
	collisable->m_position = movable->m_position - collisable->m_size/2.f;
	collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
														  collisable->m_position,
														  collisable->m_size,
														  glm::vec4(1.f, 1.f, 1.f, 1.f));
	entity->addComponent<Collisable<Cobble>>(collisable);

	AIPtr ai = std::make_shared<AI>();
	entity->addComponent<AI>(ai);

	KillablePtr life = std::make_shared<Killable>();
	life->m_defense = 10.f;
	life->m_life = 100.f;
	entity->addComponent<Killable>(life);

	PhysicPtr physic = std::make_shared<Physic>();
	physic->m_gravity = 0.0f;
	physic->m_mass = 15.f;
	physic->m_jump = false;
	entity->addComponent<Physic>(physic);

	entity_manager.add(entity);
}

Scene::Scene() {
	m_dungeon = std::make_shared<Dungeon>(generator, 20, m_entitys);
	m_player = std::make_shared<Player>(m_dungeon, m_entitys);

	ShaderProgramPtr textured = std::make_shared<ShaderProgram>("../shaders/textured.vert", "../shaders/textured.frag");
	ShaderProgramPtr billboard = std::make_shared<ShaderProgram>("../shaders/billboard.vert", "../shaders/billboard.frag");
	
	std::vector<Room<std::normal_distribution<float>, std::normal_distribution<float>>> rooms = m_dungeon->getRooms();
	for(auto& room : rooms) {
		/*EntityPtr mesh = std::make_shared<Entity>();
		RenderablePtr<MeshOBJ> cube = std::make_shared<Renderable<MeshOBJ>>(textured, "cube", "layout cube");
		RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
		renderable_component->m_renderable = cube;

		cube->scaleLocalMatrix(glm::vec3(0.07f));
		cube->translateLocalMatrix(glm::vec3(room.getCenter().x, 1.f, room.getCenter().z));


		mesh->addComponent<RenderableComponent>(renderable_component);
		m_entitys.add(mesh);*/

		EntityPtr light = std::make_shared<Entity>();
		LightPtr<Ponctual> l = std::make_shared<Light<Ponctual>>();
		l->m_position = glm::vec3(room.getCenter().x, 1.0f, room.getCenter().z);
		
		l->m_ambiant = glm::vec3(0.1f, 0.5f, 0.5f);
		l->m_diffuse = glm::vec3(1.0f, 1.0f, 1.f);
		l->m_specular = glm::vec3(1.0f, 1.0f, 1.f);

		l->m_constant = 0.01f;
		l->m_linear = 0.02f;
		l->m_quadratic = 0.03f;
		light->addComponent<Light<Ponctual>>(l);
		m_entitys.add(light);
	}

	/*EntityPtr grunt = std::make_shared<Entity>();
	RenderablePtr<Billboard> sprite = std::make_shared<Renderable<Billboard>>(billboard, "wall", glm::vec3(48.f, 0.f, 48.f), glm::vec2(100, 300));
	RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
	renderable_component->m_renderable = sprite;
	grunt->addComponent<RenderableComponent>(renderable_component);
	m_entitys.add(grunt);*/

	//Skybox
	m_skybox = std::make_shared<Renderable<Skybox>>(shaders.get("skybox"));
	//m_skybox->scaleHeritanceMatrix(glm::vec3(20.f));

	EntityPtr b = std::make_shared<Entity>();
	RenderablePtr<Box> bb = std::make_shared<Renderable<Box>>(shaders.get("wireframe"), 
																glm::vec3(0.f),
																glm::vec3(2.f),
																glm::vec4(1.0f, 0.f, 0.f, 1.f));
	RenderableComponentPtr render = std::make_shared<RenderableComponent>();
	render->m_renderable = bb;
	b->addComponent<RenderableComponent>(render);
	m_entitys.add(b);

	//Draw Box around collisable<cobble>
	/*std::vector<EntityPtr> entitys = m_entitys.getEntitys();
	for(auto& entity : entitys) {
		CollisablePtr<Cobble> collisable = entity->getComponent<Collisable<Cobble>>();
		if(collisable != nullptr) {
			EntityPtr AABB = std::make_shared<Entity>();
			
			RenderableComponentPtr renderable_component = std::make_shared<RenderableComponent>();
			renderable_component->m_renderable = box;
			AABB->addComponent<RenderableComponent>(renderable_component);

			m_entitys.add(AABB);
		}
	}*/
	
	camera->setPlayer(m_player);
	m_magic.setPlayer(m_player->getEntity());
	m_physic.setPlayer(m_player->getEntity());
	m_collided.setPlayer(m_player->getEntity());
	m_interaction.setPlayer(m_player->getEntity());
	m_motion.setPlayer(m_player->getEntity());
	m_ai.setPlayer(m_player->getEntity());

	m_renderer.setSkybox(m_skybox);

	unsigned int num_platform = 5;

	for(int i = 0; i < num_platform; ++i) {
		EntityPtr platform = std::make_shared<Entity>();
		MovablePtr movable = std::make_shared<Movable>();

		movable->m_direction = glm::vec3(glm::pow(-1.f, static_cast<float>(i)), 0.f, 0.f);
		movable->m_position = glm::vec3(22.0f, -0.25f, 20.f + i*1.0f);
		movable->m_speed = 5.0f;
		platform->addComponent<Movable>(movable);

		PhysicPtr physic = std::make_shared<Physic>();
		physic->m_gravity = 9.81f;
		physic->m_mass = 0.f;
		physic->m_jump = false;
		platform->addComponent<Physic>(physic);

		MotionPtr<LineUpDown> motion = std::make_shared<Motion<LineUpDown>>();
		motion->m_first = glm::vec3(20.f, -0.25f, 20.f + i*1.0f);
		motion->m_second = glm::vec3(25.f, -0.25f, 20.f + i*1.0f);
		motion->m_first_to_second = std::rand() % 2;
		platform->addComponent<Motion<LineUpDown>>(motion);

		CollisablePtr<Cobble> collisable = std::make_shared<Collisable<Cobble>>();
		collisable->m_position = glm::vec3(movable->m_position.x - 0.5f, -0.5f, movable->m_position.z - 0.5f);
		collisable->m_size = glm::vec3(1.0f, 0.5f, 1.0f);
		collisable->m_box = std::make_shared<Renderable<Box>>(shaders.get("wireframe"),
															  collisable->m_position,
															  collisable->m_size,
															  glm::vec4(0.f, 1.f, 1.f, 1.f));
		platform->addComponent<Collisable<Cobble>>(collisable);

		m_entitys.add(platform);
	}

	createAICharacter(m_dungeon, m_entitys);

	/*EntityPtr entity = std::make_shared<Entity>();
	RenderablePtr<MeshOBJ> goblin_mesh = std::make_shared<Renderable<MeshOBJ>>(shaders.get("textured"), "cube", "wall");
	RenderableComponentPtr render = std::make_shared<RenderableComponent>();
	goblin_mesh->scaleLocalMatrix(glm::vec3(1.0f, 1.0f, 1.0f));
	goblin_mesh->translateHeritanceMatrix(glm::vec3(3.0f, 0.f, 1.0f));
	render->m_renderable = goblin_mesh;
	entity->addComponent<RenderableComponent>(render);

	System<Collider>::attachBoundingBoxCollision(entity);
	m_entitys.add(entity);*/

	// Systems initialization
	m_collider.setDungeon(m_dungeon);
	m_collider.initialize_hashmap(m_entitys.getEntitys());

	// Creation Vertex Array
	glGenVertexArrays(1, &m_vertex_array_id);
	// Bind Vertex Array
	glBindVertexArray(m_vertex_array_id);

	// Back faces culling
	glCullFace(GL_BACK);

	//m_time = 0;
	std::cout << std::endl;
}

Scene::~Scene() {
	glDeleteVertexArrays(1, &m_vertex_array_id);
}

void Scene::run() {
	std::vector<EntityPtr>& entitys = m_entitys.getEntitys();

	m_magic.run(entitys);
	m_ai.run(entitys);
	m_motion.run(entitys);
	m_interaction.run(entitys);
	
	m_physic.run(entitys);

	m_collider.run(entitys);
	m_collided.run(entitys);

	camera->update();

	m_renderer.run(entitys);
}