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

#define SIZE_TILE 48

// SDL2 Headers
#include <SDL2/SDL.h>
#include "../include/Scene.hpp"
#include "../include/Resource.hpp"
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
		1920, 1080, SDL_WINDOW_OPENGL);

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

	// 3.2 is part of the modern versions of OpenGL, but most video cards whould be able to run it
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

	textures.add("../data/textures/floor.bmp", "stone");
	textures.add("../data/textures/wall.bmp", "wall");
	textures.add("../data/textures/wall.bmp", "layout cube");
	textures.add("../data/textures/axe_uv.bmp", "layout axe");

	textures.add("../data/textures/front.bmp", "front");
	textures.add("../data/textures/left.bmp", "left");
	textures.add("../data/textures/right.bmp", "right");
	textures.add("../data/textures/back.bmp", "back");
	textures.add("../data/textures/top.bmp", "top");
	textures.add("../data/textures/bottom.bmp", "bottom");

	meshes.add("../data/meshes/cube.obj", "cube");
	meshes.add("../data/meshes/axe.obj", "axe");

	shaders.add("../shaders/textured", "textured");
	shaders.add("../shaders/billboard", "billboard");
	shaders.add("../shaders/2d", "wireframe");
	shaders.add("../shaders/skybox", "skybox");

	RunGame();

	Cleanup();

	return 0;
}

void RunGame()
{
	Scene scene;

	bool loop = true;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (loop)
	{
		SDL_PumpEvents();
		const Uint8 *keystate = SDL_GetKeyboardState(NULL);
		
		if(keystate[SDL_SCANCODE_ESCAPE]) {
			loop = false;
		}

		scene.run();

		SDL_GL_SwapWindow(mainWindow);
	}
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