#ifndef __GAME_RENDERER_OPENGL__
#define __GAME_RENDERER_OPENGL__

// NOTE(Momo): Opengl code for renderering the game to be used by platform
// We will only have 1 model (a rectangle), which will have the 
// capabilities have having 1 texture, 1 color, and 1 transform.

#include "ryoji_maths.h"

// NOTE(Momo): Buffers
#define Vbo_Model 0 
#define Vbo_Indices 1
#define Vbo_Colors 2
#define Vbo_Texture 3
#define Vbo_Transform 4
#define Vbo_Max 5

// NOTE(Momo): Attributes
#define Atb_Model 0
#define Atb_Colors 1
#define Atb_Texture 2  
#define Atb_Transform1 3  
#define Atb_Transform2 4
#define Atb_Transform3 5
#define Atb_Transform4 6

// NOTE(Momo): VAO bindings
#define VaoBind_Model 0 
#define VaoBind_Colors 1
#define VaoBind_Texture 2
#define VaoBind_Transform 3

struct renderer_opengl {
    GLuint Buffers[Vbo_Max]; 
    GLuint Shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint Blueprint; 
    GLsizei MaxEntities;
    
    // TODO(Momo): Proper texture management
    GLuint texture;
};


static inline void 
GlAttachShader(GLuint program, GLenum type, const GLchar* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

static inline bool
Init(renderer_opengl* Opengl, GLuint Width, GLuint Height, GLsizei MaxEntities, loaded_bitmap* Bmp) 
{
    Opengl->MaxEntities = MaxEntities;
    
    constexpr static f32 quadModel[] = {
        // position   
        1.f,  1.f, 0.0f,  // top right
        1.f, -1.f, 0.0f,  // bottom right
        -1.f, -1.f, 0.0f,  // bottom left
        -1.f,  1.f, 0.0f   // top left 
    };
    
    constexpr static u8 quadIndices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    
    constexpr static f32 quadTexCoords[] = {
        1.f, 1.f, // top right
        1.f, 0.f, // bottom right
        0.f, 0.f, // bottom left
        0.f, 1.f,  // top left
    };
    
    
    // NOTE(Momo): No real need to load these from file, since we fixed
    // our pipeline.
    constexpr static const char* vertexShader = "\n\
#version 450 core \n\
layout(location=0) in vec3 aModelVtx; \n\
layout(location=1) in vec4 aColor;\n\
layout(location=2) in vec2 aTexCoord;\n\
layout(location=3) in mat4 aTransform;\n\
out vec4 mColor;\n\
out vec2 mTexCoord;\n\
uniform mat4 uProjection;\n\
\n\
void main(void) {\n\
gl_Position = uProjection * aTransform *  vec4(aModelVtx, 1.0);\n\
mColor = aColor;\n\
mTexCoord = aTexCoord;\n\
}";
    
    constexpr static const char* fragmentShader = "\
#version 450 core\n\
out vec4 fragColor;\n\
in vec4 mColor;\n\
in vec2 mTexCoord;\n\
uniform sampler2D uTexture;\n\
\n\
void main(void) {\n\
fragColor = texture(uTexture, mTexCoord) * mColor; \n\
}";
    
#if SLOW_MODE
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    
    // NOTE(Momo): Set clear color and viewport
    glViewport(0,0, Width, Height);
    glClearColor(0.0f, 0.3f, 0.3f, 0.0f);
    
    // NOTE(Momo): Setup VBO
    glCreateBuffers(Vbo_Max, Opengl->Buffers);
    glNamedBufferStorage(Opengl->Buffers[Vbo_Model], sizeof(quadModel), quadModel, 0);
    glNamedBufferStorage(Opengl->Buffers[Vbo_Indices], sizeof(quadIndices), quadIndices, 0);
    glNamedBufferStorage(Opengl->Buffers[Vbo_Texture], sizeof(quadTexCoords), quadTexCoords, 0);
    
    
    // NOTE(Momo): colors are 4x vec4, one for each side
    glNamedBufferStorage(Opengl->Buffers[Vbo_Colors], sizeof(f32)*16 * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    glNamedBufferStorage(Opengl->Buffers[Vbo_Transform], sizeof(f32)*16 * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    glCreateVertexArrays(1, &Opengl->Blueprint);
    glVertexArrayVertexBuffer(Opengl->Blueprint, VaoBind_Model, Opengl->Buffers[Vbo_Model],   0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(Opengl->Blueprint, VaoBind_Texture, Opengl->Buffers[Vbo_Texture], 0, sizeof(f32)*2);
    glVertexArrayVertexBuffer(Opengl->Blueprint, VaoBind_Colors, Opengl->Buffers[Vbo_Colors],  0, sizeof(f32)*4);
    glVertexArrayVertexBuffer(Opengl->Blueprint, VaoBind_Transform, Opengl->Buffers[Vbo_Transform], 0, sizeof(m44f));
    
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Model); 
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Model, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Model, VaoBind_Model);
    
    // aColor
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Colors); 
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Colors, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Colors, VaoBind_Colors);
    
    // NOTE(Momo): If binding divisor is zero, advance once per vertex
    glVertexArrayBindingDivisor(Opengl->Blueprint, VaoBind_Colors, 0); 
    
    // aTexCoord
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Texture); 
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Texture, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Texture, VaoBind_Texture);
    
    
    // aTransform
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Transform1); 
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Transform1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 0 * 4);
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Transform2);
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Transform2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Transform3); 
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Transform3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(Opengl->Blueprint, Atb_Transform4); 
    glVertexArrayAttribFormat(Opengl->Blueprint, Atb_Transform4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Transform1, VaoBind_Transform);
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Transform2, VaoBind_Transform);
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Transform3, VaoBind_Transform);
    glVertexArrayAttribBinding(Opengl->Blueprint, Atb_Transform4, VaoBind_Transform);
    
    // NOTE(Momo): If binding divisor is N, advance N per instance
    glVertexArrayBindingDivisor(Opengl->Blueprint, VaoBind_Transform, 1); 
    
    
    // NOTE(Momo): Setup indices
    glVertexArrayElementBuffer(Opengl->Blueprint, Opengl->Buffers[Vbo_Indices]);
    
    // Setup Shader Program
    Opengl->Shader = glCreateProgram();
    GlAttachShader(Opengl->Shader, GL_VERTEX_SHADER, vertexShader);
    GlAttachShader(Opengl->Shader, GL_FRAGMENT_SHADER, fragmentShader);
    glLinkProgram(Opengl->Shader);
    GLint result;
    glGetProgramiv(Opengl->Shader, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[KILOBYTE];
        glGetProgramInfoLog(Opengl->Shader, KILOBYTE, nullptr, msg);
        SDL_Log("Linking program failed:\n %s\n", msg);
        return false;
    }
    
    // NOTE(Momo): Setup Orthographic Projection
    GLint uProjectionLoc = glGetUniformLocation(Opengl->Shader, "uProjection");
    auto uProjection  = MakeOrthographicMatrix(-1.f, 1.f,
                                               -1.f, 1.f,
                                               -1.f, 1.f,
                                               -(f32)Width * 0.5f,  
                                               (f32)Width * 0.5f, 
                                               -(f32)Height * 0.5f, 
                                               (f32)Height* 0.5f,
                                               -100.f, 100.f,
                                               true);
    uProjection = Transpose(uProjection);
    glProgramUniformMatrix4fv(Opengl->Shader, uProjectionLoc, 1, GL_FALSE, *uProjection.Arr);
    
    
    // TODO(Momo): Do proper texture management?
    glCreateTextures(GL_TEXTURE_2D, 1, &Opengl->texture);
    glTextureStorage2D(Opengl->texture, 1, GL_RGBA8, Bmp->Width, Bmp->Height);
    
    glTextureSubImage2D(Opengl->texture, 0, 0, 0, Bmp->Width, Bmp->Height, GL_RGBA, GL_UNSIGNED_BYTE, Bmp->Pixels);
    
    Bmp->Pixels = nullptr;
    
    return 0;
}

static inline void
Render(renderer_opengl* Opengl, render_info* RenderInfo) {
    for (int i = 0; i < RenderInfo->Count; ++i) {
        glNamedBufferSubData(Opengl->Buffers[Vbo_Colors], i * sizeof(m44f), sizeof(m44f), &RenderInfo->Colors[i]);
        glNamedBufferSubData(Opengl->Buffers[Vbo_Transform], i * sizeof(m44f), sizeof(m44f), &RenderInfo->Transforms[i]);
        
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(Opengl->Shader);
    glBindTexture(GL_TEXTURE_2D, Opengl->texture);
    glBindVertexArray(Opengl->Blueprint);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, Opengl->MaxEntities);
    
}



#undef Vbo_Model
#undef Vbo_Indices
#undef Vbo_Colors
#undef Vbo_Texture
#undef Vbo_Transform
#undef Vbo_Max

#undef Atb_Model
#undef Atb_Colors
#undef Atb_Texture 

#undef Atb_Transform1 
#undef Atb_Transform2 
#undef Atb_Transform3 
#undef Atb_Transform4 

#undef VaoBind_Model  
#undef VaoBind_Colors 
#undef VaoBind_Texture 
#undef VaoBind_Transform 

#endif
