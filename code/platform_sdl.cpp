#include <stdlib.h>

#include "game.h"
#include "ryoji_maths.h"

#include "thirdparty/sdl2/include/SDL.h"
#include "thirdparty/glad/glad.c"

#include "bmp.cpp"

#include "platform_sdl_timer.cpp"
#include "platform_sdl_gldebug.cpp"

static bool gIsRunning = true;

static inline 
sdl_window_size SDLGetWindowSize(SDL_Window* Window) {
    i32 w, h;
    SDL_GetWindowSize(Window, &w, &h);
    return { w, h };
}




static inline void 
GLAttachShader(GLuint program, GLenum type, const GLchar* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}


// TODO(Momo): export as function pointer to game code
void PlatformLog(const char * str, ...) {
    va_list va;
    va_start(va, str);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, str, va);
    va_end(va);
    
}

#if 0
static
PlatformGetFileSizeRes
PlatformGetFileSize(const char* path) {
    SDL_RWops * file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return { false, 0 };
    }
    Defer {
        SDL_RWclose(file);
    };
    
    Sint64 filesize = SDL_RWsize(file);
    if(filesize < 0) {
        return { false, 0 };
    }
    u64 ret = (u64)filesize; 
    
    return { true, ret };
}

static
bool PlatformReadBinaryFileToMemory(void * dest, u64 destSize, const char * path) {
    SDL_RWops * file = SDL_RWFromFile(path, "rb");
    if (file == nullptr) {
        return false;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    u64 filesize = SDL_RWsize(file); // Does not include EOF
    
    if (filesize > destSize) {
        return false;
    }
    
    SDL_RWread(file, dest, 1, filesize);
    
    return true;
}
#endif



static inline
bool ReadFileStr(char* dest, u64 destSize, const char * path) {
    SDL_RWops* file = SDL_RWFromFile(path, "r");
    if (file == nullptr) {
        return false;
    }
    Defer{
        SDL_RWclose(file);
    };
    
    // Get file size
    u64 filesize = SDL_RWsize(file); // Does not include EOF
    
    if ((filesize + 1) > destSize) {
        return false;
    }
    
    SDL_RWread(file, dest, sizeof(char), filesize);
    
    // Don't forget null terminating value
    dest[filesize] = 0;
    
    return true;
}

// NOTE(Momo): sdl_game_code
struct sdl_game_code {
    game_update* Update;
};

static inline void
Unload(sdl_game_code* GameCode) {
    GameCode->Update = nullptr;
}

static inline bool
Load(sdl_game_code* GameCode)
{
    Unload(GameCode);
    
    void* GameCodeDLL = SDL_LoadObject("game.dll");
    if (!GameCodeDLL) {
        SDL_Log("Failed to open game.dll");
        return false;
    }
    
    if (auto fn =  (game_update*)SDL_LoadFunction(GameCodeDLL, "GameUpdate")) 
        GameCode->Update = fn;
    
    return true;
}


// NOTE(Momo): entry point
int main(int argc, char* argv[]) {
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
    
#if SLOW_MODE
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif  
    
    
    sdl_game_code GameCode;
    Load(&GameCode);
    
    // Request an OpenGL 4.5 context (should be core)
    SDL_Log("SDL creating context\n");
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr) { 
        SDL_Log("Failed to create OpenGL context! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    Defer {
        SDL_Log("SDL deleting context\b");
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
    
#ifdef SLOW_MODE
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(SdlGlDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif
    
    auto [windowWidth, windowHeight] = SDLGetWindowSize(window);
    glViewport(0,0,windowWidth,windowHeight);
    glClearColor(0.0f, 0.3f, 0.3f, 0.0f);
    
    
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
    
    f32 quadTexCoords[] = {
        0.f, 0.f, // bottom left
        1.f, 0.f, // bottom right
        1.f, 1.f, // top right
        0.f, 1.f  // top left
    };
    
    constexpr u32 kMaxEntities = 361;
    
    // TODO(Momo): Shift this to game code?
    enum  {
        VBO_MODEL,
        VBO_INDICES,
        VBO_COLORS,
        VBO_TEX_COORDS,
        VBO_INSTANCE_TRANSFORM,
        VBO_MAX
    };
    
    enum {
        ATTRIB_MODEL,
        ATTRIB_COLORS,
        ATTRIB_TEX_COORDS,
        ATTRIB_INSTANCE_TRANSFORM1,
        ATTRIB_INSTANCE_TRANSFORM2,
        ATTRIB_INSTANCE_TRANSFORM3,
        ATTRIB_INSTANCE_TRANSFORM4
    };
    
    enum {
        VAO_BIND_MODEL,
        VAO_BIND_COLORS,
        VAO_BIND_TEX_COORDS,
        VAO_BIND_INSTANCE_TRANSFORM,
    };
    
    // Setup Textures
    Bmp bmp;
    if (auto err = Load(&bmp, "assets/ryoji.bmp"); err > 0) {
        SDL_Log("%s", BmpErrorStr(err));
        return 1;
    }
    Defer{ Unload(&bmp); };
    
    
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, bmp.InfoHeader.Width, bmp.InfoHeader.Height);
    
    glTextureSubImage2D(texture, 0, 0, 0, bmp.InfoHeader.Width, bmp.InfoHeader.Height, GL_RGBA, GL_UNSIGNED_BYTE, 
                        bmp.Pixels);
    
    // Setup VBO
    GLuint vbos[VBO_MAX]; 
    glCreateBuffers(VBO_MAX, vbos);
    glNamedBufferStorage(vbos[VBO_MODEL], sizeof(quadModel), quadModel, 0);
    glNamedBufferStorage(vbos[VBO_INDICES], sizeof(quadIndices), quadIndices, 0);
    glNamedBufferStorage(vbos[VBO_COLORS], sizeof(quadColorful), quadColorful, 0);
    glNamedBufferStorage(vbos[VBO_TEX_COORDS], sizeof(quadTexCoords), quadTexCoords, 0);
    glNamedBufferStorage(vbos[VBO_INSTANCE_TRANSFORM], sizeof(m44f) * kMaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // Setup VAO
    GLuint vaos;
    glCreateVertexArrays(1, &vaos);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_MODEL,  vbos[VBO_MODEL],   0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_COLORS,  vbos[VBO_COLORS],  0, sizeof(f32)*4);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_TEX_COORDS, vbos[VBO_TEX_COORDS], 0, sizeof(f32) * 2);
    glVertexArrayVertexBuffer(vaos, VAO_BIND_INSTANCE_TRANSFORM, vbos[VBO_INSTANCE_TRANSFORM], 0, sizeof(m44f));
    
    
    // Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(vaos, ATTRIB_MODEL); 
    glVertexArrayAttribFormat(vaos, ATTRIB_MODEL, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaos, ATTRIB_MODEL, VAO_BIND_MODEL);
    
    // aColor
    glEnableVertexArrayAttrib(vaos, ATTRIB_COLORS); 
    glVertexArrayAttribFormat(vaos, ATTRIB_COLORS, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaos, ATTRIB_COLORS, VAO_BIND_COLORS);
    
    // aTexCoord
    glEnableVertexArrayAttrib(vaos, ATTRIB_TEX_COORDS); 
    glVertexArrayAttribFormat(vaos, ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaos, ATTRIB_TEX_COORDS, VAO_BIND_TEX_COORDS);
    
    
    // aInstanceTf
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM1); // aInstanceTf
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM1, 4, GL_FLOAT, GL_FALSE, 0);
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM2);
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM3); 
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(vaos, ATTRIB_INSTANCE_TRANSFORM4); 
    glVertexArrayAttribFormat(vaos, ATTRIB_INSTANCE_TRANSFORM4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM1, VAO_BIND_INSTANCE_TRANSFORM);
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM2, VAO_BIND_INSTANCE_TRANSFORM);
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM3, VAO_BIND_INSTANCE_TRANSFORM);
    glVertexArrayAttribBinding(vaos, ATTRIB_INSTANCE_TRANSFORM4, VAO_BIND_INSTANCE_TRANSFORM);
    
    // And this one 1 time, not 4 times? Makes sense I think??
    glVertexArrayBindingDivisor(vaos, VAO_BIND_INSTANCE_TRANSFORM, 1); 
    
    // Indices
    glVertexArrayElementBuffer(vaos, vbos[VBO_INDICES]);
    
    
    // Setup Shader Program
    GLuint program = glCreateProgram();
    
    // Setup Vertex Shader
    char buffer[KILOBYTE] = {};
    if (!ReadFileStr(buffer, KILOBYTE, "shader/simple.vts")) {
        SDL_Log("Loading simple.vts failed");
        return 1;
    }
    GLAttachShader(program, GL_VERTEX_SHADER, buffer);
    memset(buffer, 0, KILOBYTE);
    
    // Setup Fragment Shader
    if (!ReadFileStr(buffer, KILOBYTE, "shader/simple.fms")) {
        SDL_Log("Loading simple.fms failed");
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
    auto uProjection  = Orthographic(-1.f, 1.f,
                                     -1.f, 1.f,
                                     -1.f, 1.f,
                                     -windowWidth * 0.5f,  windowWidth * 0.5f, 
                                     -windowHeight * 0.5f, windowHeight * 0.5f,
                                     -100.f, 100.f,
                                     true);
    uProjection = Transpose(uProjection);
    
    glProgramUniformMatrix4fv(program, uProjectionLoc, 1, GL_FALSE, *uProjection.Arr);
    
    
    // NOTE(Momo): Game Init
    game_memory GameMemory = {};
    GameMemory.IsInitialized = false;
    GameMemory.PermanentStore = malloc(Megabytes(64));
    
    if ( !GameMemory.PermanentStore ) {
        SDL_Log("Cannot allocate for PermanentStore");
        return 1;
    }
    Defer { free(GameMemory.PermanentStore); };
    
    GameMemory.PermanentStoreSize = Megabytes(64);
    
    // NOTE(Momo): PlatformAPI
    platform_api PlatformAPI = {};
    PlatformAPI.Log = PlatformLog;
    
    
    sdl_timer timer;
    Start(&timer);
    f32 rotation = 0.f;
    
    // NOTE(Momo): Game Loop
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
        
        
        // NOTE(Momo): Test Update Code
        m44f instanceTransforms[kMaxEntities];
        f32 startX = -windowWidth/2.f;
        f32 startY = -windowHeight/2.f;
        f32 xOffset = 200.f;
        f32 yOffset = 200.f;
        f32 currentXOffset = 0.f;
        f32 currentYOffset = 0.f;
        for (int i = 0; i < kMaxEntities; ++i) {
            instanceTransforms[i] = 
                Translation(startX + currentXOffset, startY + currentYOffset, 0.f) *
                RotationZ(rotation) *
                Scale(100.f, 100.f, 1.f);
            instanceTransforms[i] = Transpose(instanceTransforms[i]);
            
            glNamedBufferSubData(vbos[VBO_INSTANCE_TRANSFORM], i * sizeof(m44f), sizeof(m44f), &instanceTransforms[i]);
            
            currentXOffset += xOffset;
            if (currentXOffset > windowWidth) {
                currentXOffset = 0.f;
                currentYOffset += yOffset;
            }
        }
        rotation += deltaTime;
        if (rotation > PIf ){
            f32 diff = rotation - PIf;
            rotation = -PIf + diff;
        }
        
        
        GameCode.Update(&GameMemory, &PlatformAPI, deltaTime);
        
        
        // Update End
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vaos);
        glDrawElementsInstanced(GL_TRIANGLES, ArrayCount(quadIndices), GL_UNSIGNED_BYTE, nullptr, kMaxEntities);
        
        // NOTE(Momo): Timer update
        Tick(&timer);
        //SDL_Log("%lld  ms\n", timeElapsed);
        SDL_GL_SwapWindow(window);
        
        
    }
    
    
    return 0;
    
}
