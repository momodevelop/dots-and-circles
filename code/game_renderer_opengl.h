#ifndef __GAME_RENDERER_OPENGL__
#define __GAME_RENDERER_OPENGL__

// NOTE(Momo): Renderer code for renderering the game to be used by platform
// We will only have 1 model (a rectangle), which will have the 
// capabilities have having 1 texture, 1 color, and 1 transform.

#include "ryoji_maths.h"
#include "game_renderer.h"

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
    
    GLuint BlankTexture;
    
    // TODO(Momo): Proper texture management
    GLuint Texture[2];
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
Init(renderer_opengl* Renderer, GLuint Width, GLuint Height, GLsizei MaxEntities, loaded_bitmap* Bmp, loaded_bitmap* Bmp2 ) 
{
    Renderer->MaxEntities = MaxEntities;
    
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
    
    constexpr static const u8  whiteBmp[] {
        255, 255, 255, 255
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
    
    
    glEnable(GL_DEPTH_TEST);
#if SLOW_MODE
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    
    // NOTE(Momo): Set clear color and viewport
    // TODO(Momo): Change according to game aspect ratio? Black pillars?
    glViewport(0,0, Width, Height);
    
    // NOTE(Momo): Setup VBO
    glCreateBuffers(Vbo_Max, Renderer->Buffers);
    glNamedBufferStorage(Renderer->Buffers[Vbo_Model], sizeof(quadModel), quadModel, 0);
    glNamedBufferStorage(Renderer->Buffers[Vbo_Indices], sizeof(quadIndices), quadIndices, 0);
    glNamedBufferStorage(Renderer->Buffers[Vbo_Texture], sizeof(quadTexCoords), quadTexCoords, 0);
    glNamedBufferStorage(Renderer->Buffers[Vbo_Colors], sizeof(c4f) * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    glNamedBufferStorage(Renderer->Buffers[Vbo_Transform], sizeof(c4f) * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    glCreateVertexArrays(1, &Renderer->Blueprint);
    glVertexArrayVertexBuffer(Renderer->Blueprint, VaoBind_Model, Renderer->Buffers[Vbo_Model], 0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(Renderer->Blueprint, VaoBind_Texture, Renderer->Buffers[Vbo_Texture], 0, sizeof(f32)*2);
    glVertexArrayVertexBuffer(Renderer->Blueprint, VaoBind_Colors, Renderer->Buffers[Vbo_Colors],  0, sizeof(c4f));
    glVertexArrayVertexBuffer(Renderer->Blueprint, VaoBind_Transform, Renderer->Buffers[Vbo_Transform], 0, sizeof(m44f));
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Model); 
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Model, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Model, VaoBind_Model);
    
    // aColor
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Colors); 
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Colors, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Colors, VaoBind_Colors);
    glVertexArrayBindingDivisor(Renderer->Blueprint, VaoBind_Colors, 1); 
    
    
    // aTexCoord
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Texture); 
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Texture, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Texture, VaoBind_Texture);
    
    
    // aTransform
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Transform1); 
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Transform1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 0 * 4);
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Transform2);
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Transform2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Transform3); 
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Transform3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(Renderer->Blueprint, Atb_Transform4); 
    glVertexArrayAttribFormat(Renderer->Blueprint, Atb_Transform4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Transform1, VaoBind_Transform);
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Transform2, VaoBind_Transform);
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Transform3, VaoBind_Transform);
    glVertexArrayAttribBinding(Renderer->Blueprint, Atb_Transform4, VaoBind_Transform);
    
    // NOTE(Momo): If binding divisor is N, advance N per instance
    glVertexArrayBindingDivisor(Renderer->Blueprint, VaoBind_Transform, 1); 
    
    
    // NOTE(Momo): Setup indices
    glVertexArrayElementBuffer(Renderer->Blueprint, Renderer->Buffers[Vbo_Indices]);
    
    // Setup Shader Program
    Renderer->Shader = glCreateProgram();
    GlAttachShader(Renderer->Shader, GL_VERTEX_SHADER, vertexShader);
    GlAttachShader(Renderer->Shader, GL_FRAGMENT_SHADER, fragmentShader);
    glLinkProgram(Renderer->Shader);
    GLint result;
    glGetProgramiv(Renderer->Shader, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[KILOBYTE];
        glGetProgramInfoLog(Renderer->Shader, KILOBYTE, nullptr, msg);
        SDL_Log("Linking program failed:\n %s\n", msg);
        return false;
    }
    
    // NOTE(Momo): Setup Orthographic Projection
    GLint uProjectionLoc = glGetUniformLocation(Renderer->Shader, "uProjection");
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
    glProgramUniformMatrix4fv(Renderer->Shader, uProjectionLoc, 1, GL_FALSE, *uProjection.Arr);
    
    
    // NOTE(Momo): Create Blank Texture for rendering textureless quads
    glCreateTextures(GL_TEXTURE_2D, 1, &Renderer->BlankTexture);
    glTextureStorage2D(Renderer->BlankTexture, 1, GL_RGBA8, 1, 1);
    glTextureSubImage2D(Renderer->BlankTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, whiteBmp);
    
    // TODO(Momo): Do proper texture management?
    glCreateTextures(GL_TEXTURE_2D, 2, Renderer->Texture);
    
    glTextureStorage2D(Renderer->Texture[0], 1, GL_RGBA8, Bmp->Width, Bmp->Height);
    glTextureSubImage2D(Renderer->Texture[0], 0, 0, 0, Bmp->Width, Bmp->Height, GL_RGBA, GL_UNSIGNED_BYTE, Bmp->Pixels);
    
    
    glTextureStorage2D(Renderer->Texture[1], 1, GL_RGBA8, Bmp2->Width, Bmp2->Height);
    glTextureSubImage2D(Renderer->Texture[1], 0, 0, 0, Bmp2->Width, Bmp2->Height, 
                        GL_RGBA, GL_UNSIGNED_BYTE, Bmp2->Pixels);
    
    return 0;
}

static inline void
Render(renderer_opengl* Renderer, render_commands* Commands) {
#if 1
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // TODO(Momo): Cater for different kinds of VAOs?
    glBindVertexArray(Renderer->Blueprint);
    glUseProgram(Renderer->Shader);
    
    
    // TODO(Momo): Better way to do this without binding texture first?
    GLuint CurrentTexture = Renderer->BlankTexture;
    glBindTexture(GL_TEXTURE_2D, Renderer->BlankTexture);
    
    
    u32 EntityCount = 0;
    u32 TypedEntityCount = 0;
    
    //enum draw_mode {
    //} DrawMode;
    
    for (render_command_header* Itr = (render_command_header*)Commands->Head; 
         Itr != nullptr; 
         Itr = Itr->Next) {
        
        render_command_header* Header = (render_command_header*)Itr;
        switch(Header->Type) {
            case RenderCommandType_Clear: {
                auto Entry = (render_command_entry_clear*)Header->Entry;
                glClearColor(Entry->Colors.Red, Entry->Colors.Green, Entry->Colors.Blue, Entry->Colors.Alpha);
            } break;
            case RenderCommandType_TexturedQuad: {
                // TODO(Momo): How to manage texture handlers??
                //auto Entry = (render_command_entry_textured_quad*)Header->Entry;
                //printf("Quads! %d\n", Entry->TextureHandle);
            } break;
            case RenderCommandType_ColoredQuad: {
                
                if (CurrentTexture != Renderer->BlankTexture) {
                    glBindTexture(GL_TEXTURE_2D, Renderer->BlankTexture);
                    CurrentTexture = Renderer->BlankTexture;
                    
                    // TODO(Momo): We need a better code structure for this.
                    glDrawElementsInstanced(GL_TRIANGLES, 
                                            6, 
                                            GL_UNSIGNED_BYTE, 
                                            nullptr, 
                                            TypedEntityCount);
                    TypedEntityCount = 0;
                }
                
                auto Entry = (render_command_entry_colored_quad*)Header->Entry;
                glNamedBufferSubData(Renderer->Buffers[Vbo_Colors], 
                                     EntityCount * sizeof(c4f),
                                     sizeof(c4f), 
                                     &Entry->Colors);
                
                m44f Transform = Transpose(Entry->Transform);
                glNamedBufferSubData(Renderer->Buffers[Vbo_Transform], 
                                     EntityCount * sizeof(m44f), 
                                     sizeof(m44f), 
                                     &Transform);
                ++EntityCount;
                ++TypedEntityCount;
            } break;
        }
        
        
        glDrawElementsInstanced(GL_TRIANGLES, 
                                6, 
                                GL_UNSIGNED_BYTE, 
                                nullptr, 
                                TypedEntityCount);
        
        
        // TODO(Momo): Invalidate the rest of the transforms that are not being used?
    }
    
#else
    // TODO(Momo): For each rennder command, do 
    for (int i = 0; i < RenderInfo->Count; ++i) {
        glNamedBufferSubData(Renderer->Buffers[Vbo_Colors], i * sizeof(m44f), sizeof(m44f), &RenderInfo->Colors[i]);
        glNamedBufferSubData(Renderer->Buffers[Vbo_Transform], i * sizeof(m44f), sizeof(m44f), &RenderInfo->Transforms[i]);
        
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(Renderer->Shader);
    glBindTexture(GL_TEXTURE_2D, Renderer->Texture[0]);
    glBindVertexArray(Renderer->Blueprint);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, Renderer->MaxEntities);
#endif
    
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
