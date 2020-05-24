#include <iostream>
#include "glad/glad.h"
#include "sdl2/SDL.h"
#include "yuu/utils.h"

#include "input.h"
#include "../states.h"


// https://antongerdelan.net/opengl/hellotriangle.html
// https://bcmpinc.wordpress.com/2015/10/07/copy-a-texture-to-screen/

namespace app::states::input {
	GLfloat quad[] = {
		-0.5,-0.5, 0.0f,
		-0.5, 0.5,0.0f,
		 0.5,-0.5, 0.0f,
		 0.5, 0.5,0.0f
	};

	void attach_shader
	(GLuint program, GLenum type, const char* code) {
		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &code, NULL);
		glCompileShader(shader);
		glAttachShader(program, shader);
		glDeleteShader(shader);
	}

	void init()
	{
		GLuint vbo;
		glCreateBuffers(1, &vbo);
		glNamedBufferStorage(vbo, sizeof(quad), quad, 0);
		int buffer_index = 0;
		
		int bufferIndex = 0;
		GLuint vao;
		glCreateVertexArrays(1, &vao);
		glVertexArrayVertexBuffer(vao, bufferIndex, vbo, 0, sizeof(GLfloat) * 3);

		int attributeIndex = 0;
		glEnableVertexArrayAttrib(vao, attributeIndex);
		glVertexArrayAttribFormat(
			vao, attributeIndex, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(
			vao, attributeIndex, bufferIndex);


		GLuint program = glCreateProgram();
		glObjectLabel(GL_PROGRAM, program, -1, "TextureCopy");


		attach_shader(program, GL_VERTEX_SHADER, R"(
		  #version 450 core
		  layout(location=0) in vec2 coord;
		  void main(void) {
			gl_Position = vec4(coord, 0.0, 1.0);
		  }
		)");

		attach_shader(program, GL_FRAGMENT_SHADER, R"(
			#version 450 core
			void main(void)	{
				gl_FragColor = vec4(1,1,0,1); 
			}
		)");

		glLinkProgram(program);
		GLint result;
		glGetProgramiv(program, GL_LINK_STATUS, &result);
		if (result != GL_TRUE) {
			char msg[10240];
			glGetProgramInfoLog(program, 10240, NULL, msg);
			fprintf(stderr, "Linking program failed:\n%s\n", msg);
			abort();
		}

		glUseProgram(program);
		glUniform1i(0, 0);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void free(InputState& state)
	{

	}

	State run(InputState& state, SDL_Window* window) {
		init();

		bool isRunning = true;
		while (isRunning) {
			using namespace ryoji::maybe;
			using namespace yuu;
			auto e = yuu::pollEvent();
			if (isYes(e)) {
				if (get(e).type == SDL_QUIT) {
					break;
				}
			}
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			SDL_GL_SwapWindow(window);
		}



		free(state);
		return State::NONE;
	}

}