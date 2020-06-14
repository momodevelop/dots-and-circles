#include <stdio.h>
#include <stdlib.h>
#include "ryoji.cpp"
#include "ryoji_maths.cpp"

//#include "ryoji_arenas.cpp"
#include "yuu_sdl2.cpp"
#include "yuu_gl.cpp"
//#include "yuu_glad_simple_modelmgr.cpp"

//#include "vigil_interface.h"

global bool gIsRunning = true;


void PrintMatrix(m4f mat) {
    SDL_Log("=== Checking Matrix=== \n");
    for (u8 i = 0; i < 4; ++i) {
        for (u8 j = 0; j < 4; ++j) {
            SDL_Log("%.2f ", mat[j+i*4]);
        }
        SDL_Log("\n");
    }
}


// NOTE(Momo): Game interface implementation
void GameLog(const char * str, ...) {
    va_list va;
    va_start(va, str);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, str, va);
    va_end(va);
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
    
#if DEBUG_OGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif // DEBUG_OGL
    
    
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
    
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    
    SDL_Log("OpenGL loaded!\n");
    SDL_Log("[OpenGL] Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("[OpenGL] Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("[OpenGL] Version:  %s\n", glGetString(GL_VERSION));
    
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_DEPTH_TEST);
    //glDisable(GL_CULL_FACE);
    
#ifdef DEBUG_OGL
    GLDebug glDebugObj;
    GLDebugInit(&glDebugObj, SDL_Log);
#endif
    
    auto [windowWidth, windowHeight] = SDLGetWindowSize(window);
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.0f, 0.3f, 0.3f, 0.0f);
    
    // TODO(Momo): Game state init here?
    
    f32 quadModel[] = {
        // position   
        1.f,  1.f, 0.0f,  // top right
        1.f, -1.f, 0.0f,  // bottom right
        -1.f, -1.f, 0.0f,  // bottom left
        -1.f,  1.f, 0.0f   // top left 
    };
    
    
    f32 quadColorful[] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.0f,
        1.f, 1.f, 1.f, 0.0f,
    };
    
    u8 quadIndices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    
    
    constexpr u32 kMaxEntities = 361;
    
    
    enum  {
        VBO_MODEL,
        VBO_INDICES,
        VBO_COLORS,
        VBO_INSTANCE_TRANSFORM,
        VBO_MAX
    };
    
    enum {
        ATTRIB_MODEL,
        ATTRIB_COLORS,
        ATTRIB_INSTANCE_TRANSFORM1,
        ATTRIB_INSTANCE_TRANSFORM2,
        ATTRIB_INSTANCE_TRANSFORM3,
        ATTRIB_INSTANCE_TRANSFORM4
    };
    
    enum {
        VAO_BIND_MODEL,
        VAO_BIND_COLORS,
        VAO_BIND_INSTANCE_TRANSFORM,
    };
    
    // Setup Textures
    //GLuint texture;
    //glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    //glTextureStorage2D(texture, 1, GL_RGBA8, 500, 500);
    
    
    /*GLuint testTexture[500*500];
    for(int i = 0; i < 500*500; ++i) {
        testTexture[i] = rand();
    }
    glTextureSubImage2D(texture, 0, 0, 0, 500, 500, GL_RGBA, GL_UNSIGNED_BYTE, testTexture);
    */
    
    
    
    // Setup VBO
    GLuint vbos[VBO_MAX]; 
    glCreateBuffers(VBO_MAX, vbos);
    glNamedBufferStorage(vbos[VBO_MODEL], sizeof(quadModel), quadModel, 0);
    glNamedBufferStorage(vbos[VBO_INDICES], sizeof(quadIndices), quadIndices, 0);
    glNamedBufferStorage(vbos[VBO_COLORS], sizeof(quadColorful), quadColorful, 0);
    glNamedBufferStorage(vbos[VBO_INSTANCE_TRANSFORM], sizeof(m4f) * kMaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // Setup VAO
    GLuint vaos;
    glCreateVertexArrays(1, &vaos);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_MODEL,  vbos[VBO_MODEL],   0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_COLORS,  vbos[VBO_COLORS],  0, sizeof(f32)*4);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_INSTANCE_TRANSFORM, vbos[VBO_INSTANCE_TRANSFORM], 0, sizeof(m4f));
    
    
    // Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(vaos, ATTRIB_MODEL); // aModelVtx
    glVertexArrayAttribFormat(vaos, ATTRIB_MODEL, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaos, ATTRIB_MODEL, VAO_BIND_MODEL);
    
    // aColor
    glEnableVertexArrayAttrib(vaos, ATTRIB_COLORS); // aColor
    glVertexArrayAttribFormat(vaos, ATTRIB_COLORS, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaos, ATTRIB_COLORS, VAO_BIND_COLORS);
    
    // aInstanceTf
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM1); // aInstanceTf
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM1, 4, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM2);
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM3); 
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM4); 
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    // Also must bind 4 times???
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM1, VAO_BIND_INSTANCE_TRANSFORM);
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM2, VAO_BIND_INSTANCE_TRANSFORM);
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM3, VAO_BIND_INSTANCE_TRANSFORM);
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM4, VAO_BIND_INSTANCE_TRANSFORM);
    
    // And this one 1 time, not 4 times? Makes sense I think??
    glVertexArrayBindingDivisor(vaos, VAO_BIND_INSTANCE_TRANSFORM, 1); 
    
    // Indices
    glVertexArrayElementBuffer(vaos, vbos[VBO_INDICES]);
    
    // Set up instance transforms for our entities (for now set to identity)
    
    
    
    
    
    // Setup Shader Program
    GLuint program = glCreateProgram();
    
    // Setup Vertex Shader
    char buffer[KILOBYTE] = {};
    if (auto err = SDLReadFile(buffer, KILOBYTE, "shader/simple.vts", false);
        err != SDLERR_NONE) {
        SDL_Log("Loading simple.vts failed: %s", SDLErrorGetMsg(err));
        return 1;
    }
    GLAttachShader(program, GL_VERTEX_SHADER, buffer);
    memset(buffer, 0, KILOBYTE);
    
    // Setup Fragment Shader
    if (auto err = SDLReadFile(buffer, KILOBYTE, "shader/simple.fms", false);
        err != SDLERR_NONE) {
        SDL_Log("Loading simple.fms failed: %s", SDLErrorGetMsg(err));
        return 1;
    }
    GLAttachShader(program, GL_FRAGMENT_SHADER, buffer);
    
    
    // Link Shaders
    glLinkProgram(program);
    GLint result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[KILOBYTE];
        glGetProgramInfoLog(program, KILOBYTE, nullptr, msg);
        SDL_Log("Linking program failed:\n %s\n", msg);
        return 1;
    }
    
    // Setup uniform variables
    GLint uProjectionLoc = glGetUniformLocation(program, "uProjection");
    auto uProjection  = CreateOrthoProjection(-1.f, 1.f,
                                              -1.f, 1.f,
                                              -1.f, 1.f,
                                              -windowWidth * 0.5f,  windowWidth * 0.5f, 
                                              -windowHeight * 0.5f, windowHeight * 0.5f,
                                              -100.f, 100.f,
                                              true);
    
    glProgramUniformMatrix4fv(program, uProjectionLoc, 1, GL_FALSE, &uProjection[0]);
    
    
    SDLTimer timer;
    Start(&timer);
    
    
    
    f32 rotation = 0.f;
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
        f32 deltaTime = timeElapsed / 1000.f;
        
        
        // TODO(Momo): Update + Render
        
        // NOTE(Momo): Test Update Code
        m4f instanceTransforms[kMaxEntities];
        f32 startX = -windowWidth/2.f;
        f32 startY = -windowHeight/2.f;
        f32 xOffset = 200.f;
        f32 yOffset = 200.f;
        f32 currentXOffset = 0.f;
        f32 currentYOffset = 0.f;
        for (int i = 0; i < kMaxEntities; ++i) {
            instanceTransforms[i] = 
                CreateTranslation(startX + currentXOffset, startY + currentYOffset, 0.f) *
                CreateRotationZ(rotation) *
                CreateScale(100.f, 100.f, 1.f);
            
            glNamedBufferSubData(vbos[VBO_INSTANCE_TRANSFORM], i * sizeof(m4f), sizeof(m4f), &instanceTransforms[i]);
            
            currentXOffset += xOffset;
            if (currentXOffset > windowWidth) {
                currentXOffset = 0.f;
                currentYOffset += yOffset;
            }
        }
        rotation += deltaTime;
        SDL_Log("%f vs %f\n", rotation, PIf);
        if (rotation > PIf ){
            f32 diff = rotation - PIf;
            rotation = -PIf + diff;
        }
        
        // Update End
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vaos);
        glDrawElementsInstanced(GL_TRIANGLES, ArrayCount(quadIndices), GL_UNSIGNED_BYTE, nullptr, kMaxEntities);
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        //SDL_Log("%lld  ms\n", timeElapsed);
        SDL_GL_SwapWindow(window);
        
        
    }
    
    
    return 0;
    
}