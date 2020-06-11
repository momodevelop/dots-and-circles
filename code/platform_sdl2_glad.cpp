#include <stdio.h>
#include "ryoji_common.cpp"
#include "ryoji_maths.cpp"

//#include "ryoji_arenas.cpp"
#include "yuu_include_sdl2_glad.cpp"

//#include "vigil_interface.h"

global bool gIsRunning = true;


// NOTE(Momo): Game interface implementation
void GameLog(const char * str, ...) {
    va_list va;
    va_start(va, str);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, str, va);
    va_end(va);
}

// NOTE(Momo): Temp code!
// Need to think of how to manage these
GLfloat data[] = {
    -0.5f, -0.5f, 0.f,
    -0.5f, 0.5f, 0.f,
    0.5f, -0.5f, 0.f,
    0.5f, 0.5f, 0.f};
GLuint VBO; 
i32 bufferIndex = 0;
GLuint VAO;

pure void GLAttachShader(GLuint program, GLenum type, const char* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}



int main(int argc, char* argv[]) {
    
    
    (void)argc;
    (void)argv;
    SDL_Log("SDL initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer{
        SDL_Log("SDL shutting down\n");
        SDL_Quit();
    };
    SDL_Log("SDL creating Window\n");
    SDL_Window* window = SDL_CreateWindow("Vigil", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          1600, 
                                          900, 
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    Defer{
        SDL_Log("SDL destroying window\n");
        SDL_DestroyWindow(window);
    };
    
    if (window == nullptr) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_GL_LoadLibrary(nullptr);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    // TODO(Momo): Debug flag?
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    
    // Request an OpenGL 4.5 context (should be core)
    SDL_Log("SDL creating context\n");
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) { 
        SDL_Log("Failed to create OpenGL context! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer {
        SDL_Log("SDL deleting context\b", SDL_GetError());
        SDL_GL_DeleteContext(context);
    };
    
    // NOTE(Momo): Check open GL properties
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    // Check OpenGL properties
    SDL_Log("OpenGL loaded!\n");
    SDL_Log("[OpenGL] Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("[OpenGL] Version:  %s\n", glGetString(GL_VERSION));
    
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
#ifdef DEBUG_OGL
    GLDebug glDebugObj;
    GLDebugInit(&glDebugObj, SDL_Log);
#endif
    
    auto [w,h] = SDLGetWindowSize(window);
    glViewport(0,0,w,h);
    glClearColor(0.0f, 0.3f, 0.3f, 0.0f);
    
    
    
    
    
    // TODO(Momo): Game state init here?
    
    // TODO(Momo):  Test code, remove later
    glCreateBuffers(1, &VBO);
    glCreateVertexArrays(1, &VAO);
    glNamedBufferStorage(VBO, sizeof(data), data, 0);
    
    i32 attributeIndex = 0;
    glEnableVertexArrayAttrib(VAO, attributeIndex);
    glVertexArrayVertexBuffer(VAO, bufferIndex, VBO, 0, sizeof(GLfloat)*3);
    glVertexArrayAttribFormat(VAO, attributeIndex, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(VAO, attributeIndex, bufferIndex);
    
    
    GLuint program = glCreateProgram();
    glObjectLabel(GL_PROGRAM, program, -1, "TextureCopy");
    
    char buffer[KILOBYTE] = {};
    
    if (auto err = SDLReadFileToString(buffer, KILOBYTE, "shader/simple.vts", false);
        err != SDLError::NONE) {
        SDL_Log("Loading simple.vts failed: %s", SDLErrorGetMsg(err));
        return 1;
    }
    GLAttachShader(program, GL_VERTEX_SHADER, buffer);
    memset(buffer, 0, KILOBYTE);
    
    
    if (auto err = SDLReadFileToString(buffer, KILOBYTE, "shader/simple.fms", false);
        err != SDLError::NONE) {
        SDL_Log("Loading simple.fms failed: %s", SDLErrorGetMsg(err));
        return 1;
    }
    GLAttachShader(program, GL_FRAGMENT_SHADER, buffer);
    
    glLinkProgram(program);
    GLint result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[KILOBYTE];
        glGetProgramInfoLog(program, KILOBYTE, nullptr, msg);
        SDL_Log("Linking program failed:\n %s\n", msg);
        return 1;
    }
    
    GLint transformLoc = glGetUniformLocation(program, "transform");
    
    
    //auto translation = GetTranslation(0.5f, 0.5f, 0.5f);
    
    SDLTimer timer;
    Start(&timer);
    
    f32 ticker = 0.f;
    while(gIsRunning) {
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT: {
                    gIsRunning = false;
                    SDL_Log("Quit triggered\n");
                } break; 
            }
        }
        
        
        u64 timeElapsed = TimeElapsed(&timer);
        float delta = (f32)timeElapsed/1000.f;
        
        auto translation = Scale(ticker, ticker,);
        ticker += delta;
        Wrap(ticker, 0.f, 2.f);
        
        
        // TODO(Momo): Update + Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(program);
        //glBindImageTexture(0, texture, 0, false, 0, GL_READ_ONLY, GL_RGBA8);
        //glUniform1i(0, 0);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, translation.arr);
        glBindVertexArray(VAO);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        SDL_Log("%lld  ms\n", timeElapsed);
        
        SDL_GL_SwapWindow(window);
        
        
    }
    
    
    return 0;
    
}