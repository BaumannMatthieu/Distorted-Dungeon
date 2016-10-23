// Headerphile.com OpenGL Tutorial part 1
// A Hello World in the world of OpenGL ( creating a simple windonw and setting background color )
// Source code is an C++ adaption / simplicication of : https://www.opengl.org/wiki/Tutorial1:_Creating_a_Cross_Platform_OpenGL_3.2_Context_in_SDL_(C_/_SDL)
// Compile : clang++ main.cpp -lGL -lSDL2 -std=c++11 -o Test

// C++ Headers
#include <string>
#include <iostream>

// OpenGL / glew Headers
#define GL3_PROTOTYPES 1
#include <GL/glew.h>

// SDL2 Headers
#include <SDL2/SDL.h>
#include "../include/Scene.hpp"
#include "../include/Resource.hpp"
#include "../include/Config.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/rotate_vector.hpp>

Resources<Texture> textures;
Resources<MeshOBJ> meshes;
Resources<ShaderProgram> shaders;

std::string programName = "Headerphile SDL2 - OpenGL thing";

// Our SDL_Window ( just like with SDL2 wihout OpenGL)
SDL_Window *mainWindow;

// Our opengl context handle
SDL_GLContext mainContext;

bool SetOpenGLAttributes();
void PrintSDL_GL_Attributes();
void CheckSDLError(int line);
void RunGame();
void Cleanup();

bool Init()
{
	// Initialize SDL's Video subsystem
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Failed to init SDL\n";
		return false;
	}

	// Create our window centered at 512x512 resolution
	mainWindow = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	SDL_SetWindowFullscreen(mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);

	// Check that everything worked out okay
	if (!mainWindow)
	{
		std::cout << "Unable to create window\n";
		CheckSDLError(__LINE__);
		return false;
	}

	// Create our opengl context and attach it to our window
	mainContext = SDL_GL_CreateContext(mainWindow);
	
	SetOpenGLAttributes();

	// This makes our buffer swap syncronized with the monitor's vertical refresh
	SDL_GL_SetSwapInterval(1);

	// Init GLEW
	// Apparently, this is needed for Apple. Thanks to Ross Vander for letting me know
	#ifndef __APPLE__
		glewExperimental = GL_TRUE;
		glewInit();
	#endif
	
	return true;
}

bool SetOpenGLAttributes()
{
	// Set our OpenGL version.
	// SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// 3.2 is part of the modern versions of OpenGL, but most video cards would be able to run it
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Turn on double buffering with a 24bit Z buffer.
	// You may need to change this to 16 or 32 for your system
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	return true;
}

int main(int argc, char *argv[])
{
	if (!Init())
		return -1;

	srand(time(NULL));

	// Clear our buffer with a black background
	// This is the same as :
	// 		SDL_SetRenderDrawColor(&renderer, 255, 0, 0, 255);
	// 		SDL_RenderClear(&renderer);
	//
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(mainWindow);

	textures.add("../data/textures/floor.jpg", "stone");
	textures.add("../data/textures/wall.jpg", "wall");
	textures.add("../data/textures/wall.jpg", "layout cube");
	textures.add("../data/textures/axe_uv.bmp", "layout axe");
	textures.add("../data/textures/goblin-texture.bmp", "goblin");
	textures.add("../data/textures/ward-texture.jpg", "ward-texture");
	textures.add("../data/textures/smoke.jpg", "smoke");
	textures.add("../data/textures/colonne.jpg", "colonne_tex");
	textures.add("../data/textures/smoke.jpg", "heart");
	textures.add("../data/textures/alagar.bmp", "alagar");
	textures.add("../data/textures/ulukai/corona_ft.png", "wallpaper");

	textures.add("../data/textures/ulukai/corona_ft.png", "front");
	textures.add("../data/textures/ulukai/corona_lf.png", "left");
	textures.add("../data/textures/ulukai/corona_rt.png", "right");
	textures.add("../data/textures/ulukai/corona_bk.png", "back");
	textures.add("../data/textures/ulukai/corona_up.png", "top");
	textures.add("../data/textures/ulukai/corona_dn.png", "bottom");

	meshes.add("../data/meshes/cube.obj", "cube");
	meshes.add("../data/meshes/axe.obj", "axe");
	meshes.add("../data/meshes/Goblin.obj", "goblin");
	meshes.add("../data/meshes/theWard.obj", "ward");
	meshes.add("../data/meshes/colonne1.obj", "colonne");

	shaders.add("../shaders/textured", "textured");
	shaders.add("../shaders/fire", "fire");
	shaders.add("../shaders/billboard", "billboard");
	shaders.add("../shaders/2d", "wireframe");
	shaders.add("../shaders/skybox", "skybox");
	shaders.add("../shaders/ui_element", "UI");
	shaders.add("../shaders/font", "font");

	RunGame();

	Cleanup();

	return 0;
}

template<typename StateType>
class GameState;

template<typename StateType>
using GameStatePtr = std::shared_ptr<GameState<StateType>>;

struct State;
using StatePtr = std::shared_ptr<State>;

struct State : public std::enable_shared_from_this<State> {
	virtual const StatePtr update() = 0;
};


template<typename StateType>
class GameState : public StateType,
				  public State {
	public:
		GameState() {
		}
		
		~GameState() {
		}

		void addNextState(const StatePtr next) {
			m_next.push_back(next);
		}

		const StatePtr update() {
			int index = StateType::update();
			if(index > -1) {
				if(index == 0) {
					return shared_from_this();
				}

				return m_next[index - 1];
			}
			return nullptr;
		}

	private:
		std::vector<StatePtr>	m_next;
};

class Game {
	public:
		enum {THIS, GAMEOVER, PAUSE};

		Game() {
		}
		~Game() {
		}

		int update() {
			SDL_PumpEvents();
			const Uint8 *keystate = SDL_GetKeyboardState(NULL);

			if(keystate[SDL_SCANCODE_ESCAPE]) {
				m_pause = true;
				return PAUSE;
			}

			if(m_scene == nullptr) {
				m_scene = std::make_shared<Scene>();
			}

			bool died = m_scene->run(m_pause);
			if(died) {
				m_scene = nullptr;
				return GAMEOVER;
			}

			m_pause = false;

			return 0;
		}

	private:
		ScenePtr 		m_scene;
		bool			m_pause;
};

class Menu {
	public:
		enum {THIS, START_GAME, CONFIG, QUIT};

		Menu() {
			if(FT_Init_FreeType(&m_ft)) {
				std::cerr << "Could not init freetype library" << std::endl;
			}

			if(FT_New_Face(m_ft, "../data/font/agalar.ttf", 0, &m_face)) {
			  	std::cerr << "Could not open font" << std::endl;
			}

			m_button_start = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f - 100.f), "Start Game !", m_face);
			m_button_conf = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), "Configuration", m_face);
			m_button_quit = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f + 100.f), "Quit", m_face);
			
			FT_Set_Pixel_Sizes(m_face, 0, 200);
			m_title = std::make_shared<Renderable<UIText>>(shaders.get("font"), "Cursed Crown", m_face, glm::vec2(WINDOW_WIDTH/2.f, 100.f));
			m_wallpaper = std::make_shared<Renderable<UIElement>>(shaders.get("UI"), "wallpaper", glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
		}

		void draw() {
			glDepthMask(GL_FALSE);
			m_wallpaper->draw(false);

			m_button_start->draw();
			m_button_conf->draw();
			m_button_quit->draw();

			m_title->draw();
			glDepthMask(GL_TRUE);
		}

		int update() {
			SDL_PumpEvents();
			const Uint8 *keystate = SDL_GetKeyboardState(NULL);

			if(m_button_start->clicked()) {
				return START_GAME;
			}

			if(m_button_conf->clicked()) {
				return 0;
			}

			if(m_button_quit->clicked()) {
				return -1;
			}

			draw();

			return 0;
		}

	private:
		FT_Library 			m_ft;
		FT_Face 			m_face;

		ButtonPtr	  		m_button_start;
		ButtonPtr	  		m_button_conf;
		ButtonPtr	  		m_button_quit;

		RenderablePtr<UIElement>	m_wallpaper;
		RenderablePtr<UIText>		m_title;
};

class GameOver {
	public:
		enum {THIS, START_NEW_GAME, MENU};

		GameOver() {
			if(FT_Init_FreeType(&m_ft)) {
				std::cerr << "Could not init freetype library" << std::endl;
			}

			if(FT_New_Face(m_ft, "../data/font/agalar.ttf", 0, &m_face)) {
			  	std::cerr << "Could not open font" << std::endl;
			}

			m_button_start_new = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f - 100.f), "Start New Game !", m_face);
			m_button_menu = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), "Back to Menu", m_face);
						
			FT_Set_Pixel_Sizes(m_face, 0, 200);
			m_title = std::make_shared<Renderable<UIText>>(shaders.get("font"), "You Died !", m_face, glm::vec2(WINDOW_WIDTH/2.f, 100.f));
			m_wallpaper = std::make_shared<Renderable<UIElement>>(shaders.get("UI"), "wallpaper", glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
		}

		void draw() {
			glDepthMask(GL_FALSE);
			m_wallpaper->draw(false);

			m_button_start_new->draw();
			m_button_menu->draw();

			m_title->draw();
			glDepthMask(GL_TRUE);
		}

		int update() {
			draw();

			SDL_PumpEvents();
			const Uint8 *keystate = SDL_GetKeyboardState(NULL);

			if(m_button_start_new->clicked()) {
				return START_NEW_GAME;
			}

			if(m_button_menu->clicked()) {
				return MENU;
			}

			return 0;
		}

	private:
		FT_Library 			m_ft;
		FT_Face 			m_face;

		ButtonPtr	  		m_button_start_new;
		ButtonPtr	  		m_button_menu;

		RenderablePtr<UIElement>	m_wallpaper;
		RenderablePtr<UIText>		m_title;
};

class Pause {
	public:
		enum {THIS, RETURN, MENU};

		Pause() {
			if(FT_Init_FreeType(&m_ft)) {
				std::cerr << "Could not init freetype library" << std::endl;
			}

			if(FT_New_Face(m_ft, "../data/font/agalar.ttf", 0, &m_face)) {
			  	std::cerr << "Could not open font" << std::endl;
			}

			m_button_return = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f - 100.f), "Return", m_face);
			m_button_menu = std::make_shared<Button>(glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), "Menu", m_face);
						
			FT_Set_Pixel_Sizes(m_face, 0, 200);
			m_title = std::make_shared<Renderable<UIText>>(shaders.get("font"), "Cursed Crown", m_face, glm::vec2(WINDOW_WIDTH/2.f, 100.f));
			//m_wallpaper = std::make_shared<Renderable<UIElement>>(shaders.get("UI"), "wallpaper", glm::vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT/2.f), glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
		}

		void draw() {
			glDepthMask(GL_FALSE);
			//m_wallpaper->draw(false);

			m_button_return->draw();
			m_button_menu->draw();

			m_title->draw();
			glDepthMask(GL_TRUE);
		}

		int update() {
			draw();

			SDL_PumpEvents();
			const Uint8 *keystate = SDL_GetKeyboardState(NULL);

			if(m_button_return->clicked()) {
				return RETURN;
			}

			if(m_button_menu->clicked()) {
				return MENU;
			}

			return 0;
		}

	private:
		FT_Library 			m_ft;
		FT_Face 			m_face;

		ButtonPtr	  		m_button_return;
		ButtonPtr	  		m_button_menu;

		RenderablePtr<UIElement>	m_wallpaper;
		RenderablePtr<UIText>		m_title;
};

void RunGame()
{
	GameStatePtr<Game> game = std::make_shared<GameState<Game>>();
	GameStatePtr<Menu> menu = std::make_shared<GameState<Menu>>();
	GameStatePtr<GameOver> over = std::make_shared<GameState<GameOver>>();
	GameStatePtr<Pause> pause = std::make_shared<GameState<Pause>>();
	game->addNextState(over);
	game->addNextState(pause);

	menu->addNextState(game);

	over->addNextState(game);
	over->addNextState(menu);

	pause->addNextState(game);
	pause->addNextState(menu);

	StatePtr current_state = menu;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	do {
		current_state = current_state->update();
		if(current_state == nullptr) {
			break;
		}
		SDL_GL_SwapWindow(mainWindow);
	} while(current_state != nullptr);
}

void Cleanup()
{
	// Delete our OpengL context
	SDL_GL_DeleteContext(mainContext);

	// Destroy our window
	SDL_DestroyWindow(mainWindow);

	// Shutdown SDL 2
	SDL_Quit();
}

void CheckSDLError(int line = -1)
{
	std::string error = SDL_GetError();

	if (error != "")
	{
		std::cout << "SLD Error : " << error << std::endl;

		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;

		SDL_ClearError();
	}
}

void PrintSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}