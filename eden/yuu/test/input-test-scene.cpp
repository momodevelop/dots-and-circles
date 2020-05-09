#include "input-test-scene.h"
#include "yuu/input/keyboard.h"
#include "glew/glew.h"
#include <optional>
#include <iostream>

using namespace scenes;
using namespace std;
using namespace yuu::input::keyboard;


float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f

};

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

unsigned int VBO;
unsigned int vertexShader;
unsigned int fragmentShader;
unsigned int shaderProgram;

// Wrapper to change the awkward way SDL_PollEvent is being called.
static std::optional<SDL_Event> pollEvent() {
	SDL_Event e;
	bool success = SDL_PollEvent(&e);
	if (success) {
		return e;
	}
	return std::nullopt;
}

void checkKeyPressed(Keyboard& kb, SDL_KeyCode key, const char* keyName) {
	using namespace yuu;
	using namespace std;
	if (isDown(kb, key))
		cout << keyName << " is down" << endl;
	if (isJustPressed(kb, key))
		cout << keyName << " is just pressed" << endl;
	if (isHeld(kb, key))
		cout << keyName << " is held" << endl;
	if (isJustReleased(kb, key))
		cout << keyName << " is just released" << endl;
}

void InputTestScene::init() {
	using namespace yuu::input;
	keyboard::registerKey(kb, SDLK_w);
	keyboard::registerKey(kb, SDLK_s);
	keyboard::registerKey(kb, SDLK_a);
	keyboard::registerKey(kb, SDLK_d);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);


	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "shader program cannot be created\n" << infoLog << std::endl;
	}
	glUseProgram(shaderProgram);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

}
IScene* InputTestScene::update() {
	using namespace yuu::input;
	keyboard::update(kb);
	while (auto e = pollEvent()) {
		if (e->type == SDL_QUIT)
			return nullptr;
		else {
			keyboard::processEvent(kb, e.value());
		}

	}
	checkKeyPressed(kb, SDLK_w, "W");
	checkKeyPressed(kb, SDLK_a, "A");
	checkKeyPressed(kb, SDLK_s, "S");
	checkKeyPressed(kb, SDLK_d, "D");

	// 0. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 1. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// 2. use our shader program when we want to render an object
	glUseProgram(shaderProgram);
	// 3. now draw the object 


	return this;
}
void InputTestScene::uninit() {}



