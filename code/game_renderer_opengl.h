#ifndef __GAME_RENDERER_OPENGL__
#define __GAME_RENDERER_OPENGL__

#include "game_renderer.h"
#include "game_assets.h"

// NOTE(Momo): Buffers
enum {
    renderer_vbo_Model,
    renderer_vbo_Indices,
    renderer_vbo_Colors,
    renderer_vbo_Texture,
    renderer_vbo_Transform,
    renderer_vbo_Max
};


// NOTE(Momo): Attributes
enum { 
    renderer_atb_Model,    // 0 
    renderer_atb_Colors,   // 1
    renderer_atb_Texture1, // 2
    renderer_atb_Texture2, // 3
    renderer_atb_Texture3, // 4
    renderer_atb_Texture4, // 5
    renderer_atb_Transform1, // 6
    renderer_atb_Transform2, // 7
    renderer_atb_Transform3, // 8
    renderer_atb_Transform4  // 9
};

// NOTE(Momo): VAO bindings
enum {
    renderer_vaobind_Model,
    renderer_vaobind_Colors,
    renderer_vaobind_Texture,
    renderer_vaobind_Transform
};

struct renderer_opengl {
    GLuint Buffers[renderer_vbo_Max]; 
    GLuint Shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint Blueprint; 
    GLsizei MaxEntities;
    
    GLuint BlankTexture;
    
    // NOTE(Momo): A table mapping 'game texture handler' <-> 'opengl texture handler'  
    GLuint GameToRendererTextureTable[GameBitmapHandle_Max] = {};
    
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
Init(renderer_opengl* Renderer, GLuint Width, GLuint Height, GLsizei MaxEntities) 
{
    Renderer->MaxEntities = MaxEntities;
    
    
    // NOTE(Momo): No real need to load these from file, since we fixed
    // our pipeline.
    constexpr static const char* vertexShader = "#version 450 core\nlayout(location=0) in vec3 aModelVtx; \nlayout(location=1) in vec4 aColor;\n\
                                                                                                                                                                                                                                                                        layout(location=2) in vec2 aTexCoord[4];\n\
                                                                                                                                                                                                                                                                                                                \nlayout(location=6) in mat4 aTransform;\nout vec4 mColor;\nout vec2 mTexCoord;\nuniform mat4 uProjection;\n\n\
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    void main(void) {\ngl_Position = uProjection * aTransform *  vec4(aModelVtx, 1.0);\nmColor = aColor;\n\
                                                                                                                                                        mTexCoord = aTexCoord[gl_VertexID];\n\
                                                                                                                                                                                     mTexCoord.y = 1.0 - mTexCoord.y;\n\
                                                                                                                                                                                                                                                                                                                                                                                }";
    
    
#if 1
    constexpr static const char* fragmentShader = "#version 450 core\nout vec4 fragColor;\nin vec4 mColor;\nin vec2 mTexCoord;\nuniform sampler2D uTexture;\nvoid main(void) {\nfragColor = texture(uTexture, mTexCoord) * mColor; \n}";
    
#else
    constexpr static const char* fragmentShader = "#version 450 core\nout vec4 fragColor;\nin vec4 mColor;\nin vec2 mTexCoord;\nuniform sampler2D uTexture;\nvoid main(void) {\n\
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    fragColor.x = mTexCoord.x; fragColor.y = mTexCoord.y; fragColor.z = 0.0; fragColor.w = 1.0; \n}";
#endif
    
    glEnable(GL_DEPTH_TEST);
    
#if INTERNAL
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    
    // NOTE(Momo): Set clear color and viewport
    // TODO(Momo): Change according to game aspect ratio? Black pillars?
    glViewport(0,0, Width, Height);
    
    // NOTE(Momo): Setup VBO
    glCreateBuffers(renderer_vbo_Max, Renderer->Buffers);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Model], sizeof(QuadModel), QuadModel, 0);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Indices], sizeof(QuadIndices), QuadIndices, 0);
    
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Texture], sizeof(v2f) * 4 * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Colors], sizeof(c4f) * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Transform], sizeof(m44f) * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    glCreateVertexArrays(1, &Renderer->Blueprint);
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Model, Renderer->Buffers[renderer_vbo_Model], 0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Texture, Renderer->Buffers[renderer_vbo_Texture], 0, sizeof(f32) * 8);
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Colors, Renderer->Buffers[renderer_vbo_Colors],  0, sizeof(c4f));
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Transform, Renderer->Buffers[renderer_vbo_Transform], 0, sizeof(m44f));
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Model); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Model, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Model, renderer_vaobind_Model);
    
    // aColor
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Colors); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Colors, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Colors, renderer_vaobind_Colors);
    glVertexArrayBindingDivisor(Renderer->Blueprint, renderer_vaobind_Colors, 1); 
    
    // aTexCoord
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Texture1); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Texture1, 2, GL_FLOAT, GL_FALSE, sizeof(v2f) * 0);
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Texture2); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Texture2, 2, GL_FLOAT, GL_FALSE, sizeof(v2f) * 1);
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Texture3); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Texture3, 2, GL_FLOAT, GL_FALSE, sizeof(v2f) * 2);
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Texture4); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Texture4, 2, GL_FLOAT, GL_FALSE, sizeof(v2f) * 3);
    
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Texture1, renderer_vaobind_Texture);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Texture2, renderer_vaobind_Texture);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Texture3, renderer_vaobind_Texture);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Texture4, renderer_vaobind_Texture);
    
    glVertexArrayBindingDivisor(Renderer->Blueprint, renderer_vaobind_Texture, 1); 
    
    
    // aTransform
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Transform1); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Transform1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 0 * 4);
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Transform2);
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Transform2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Transform3); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Transform3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(Renderer->Blueprint, renderer_atb_Transform4); 
    glVertexArrayAttribFormat(Renderer->Blueprint, renderer_atb_Transform4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Transform1, renderer_vaobind_Transform);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Transform2, renderer_vaobind_Transform);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Transform3, renderer_vaobind_Transform);
    glVertexArrayAttribBinding(Renderer->Blueprint, renderer_atb_Transform4, renderer_vaobind_Transform);
    glVertexArrayBindingDivisor(Renderer->Blueprint, renderer_vaobind_Transform, 1); 
    
    // NOTE(Momo): Alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // NOTE(Momo): Setup indices
    glVertexArrayElementBuffer(Renderer->Blueprint, Renderer->Buffers[renderer_vbo_Indices]);
    
    
    // Setup Shader Program
    Renderer->Shader = glCreateProgram();
    GlAttachShader(Renderer->Shader, GL_VERTEX_SHADER, vertexShader);
    GlAttachShader(Renderer->Shader, GL_FRAGMENT_SHADER, fragmentShader);
    glLinkProgram(Renderer->Shader);
    GLint result;
    glGetProgramiv(Renderer->Shader, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[Kilobyte];
        glGetProgramInfoLog(Renderer->Shader, Kilobyte, nullptr, msg);
        // TODO(Momo): Log?
        return false;
    }
    
    // NOTE(Momo): Setup Orthographic Projection
    GLint uProjectionLoc = glGetUniformLocation(Renderer->Shader, "uProjection");
    auto uProjection  = OrthographicMatrix(-1.f, 1.f,
                                           -1.f, 1.f,
                                           -1.f, 1.f,
                                           -(f32)Width * 0.5f,  
                                           (f32)Width * 0.5f, 
                                           -(f32)Height * 0.5f, 
                                           (f32)Height* 0.5f,
                                           -100.f, 100.f,
                                           true);
    uProjection = Transpose(uProjection);
    glProgramUniformMatrix4fv(Renderer->Shader, uProjectionLoc, 1, GL_FALSE, uProjection[0]);
    
    return true;
}

static inline void
Render(renderer_opengl* Renderer, render_commands* Commands) 
{
    // TODO(Momo): Better way to do this without binding texture first?
    GLuint CurrentTexture = 0;
    u32 InstancesToDrawCount = 0;
    u32 LastDrawnInstanceIndex = 0;
    u32 CurrentInstanceIndex = 0;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (u32 i = 0; i < Commands->EntryCount; ++i) {
        auto* Entry = GetEntry(Commands, i);
        
        switch(Entry->Type) {
            case render_command_data_clear::TypeId: {
                using data_t = render_command_data_clear;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                glClearColor(Data->Colors.R, Data->Colors.G, Data->Colors.B, Data->Colors.A);
            } break;
            case render_command_data_link_texture::TypeId: {
                using data_t = render_command_data_link_texture;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                
                if (Renderer->GameToRendererTextureTable[Data->TextureHandle] != 0) {
                    // TODO(Momo): unload and reload texture
                }
                GLuint* const TextureTableEntry = &Renderer->GameToRendererTextureTable[Data->TextureHandle];
                glCreateTextures(GL_TEXTURE_2D, 1, TextureTableEntry);
                glTextureStorage2D((*TextureTableEntry), 1, GL_RGBA8, Data->TextureBitmap.Width, Data->TextureBitmap.Height);
                glTextureSubImage2D((*TextureTableEntry), 
                                    0, 0, 0, 
                                    Data->TextureBitmap.Width, Data->TextureBitmap.Height, 
                                    GL_RGBA, GL_UNSIGNED_BYTE, 
                                    Data->TextureBitmap.Pixels);
                
            } break;
            case render_command_data_textured_quad::TypeId: {
                using data_t = render_command_data_textured_quad;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                
                // If the game texture handle does not exist in the lookup table, add texture to renderer and register it into the lookup table
                u32 GameBitmapHandle = Data->TextureHandle;
                GLuint RendererTextureHandle = Renderer->GameToRendererTextureTable[GameBitmapHandle];
                
                if (RendererTextureHandle == 0) {
                    // TODO(Momo): Maybe render a dummy texture?
                    Assert(false);
                }
                
                // NOTE(Momo): If the currently set texture is not same as the currently processed texture, batch draw all instances before the current instance.
                if (CurrentTexture != RendererTextureHandle) {
                    if (InstancesToDrawCount != 0) {
                        glBindTexture(GL_TEXTURE_2D, CurrentTexture);
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        
                        glBindVertexArray(Renderer->Blueprint);
                        glUseProgram(Renderer->Shader);
                        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                                            6, 
                                                            GL_UNSIGNED_BYTE, 
                                                            nullptr, 
                                                            InstancesToDrawCount,
                                                            LastDrawnInstanceIndex);
                        LastDrawnInstanceIndex += InstancesToDrawCount;
                        InstancesToDrawCount= 0;
                    }
                    CurrentTexture = RendererTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                glNamedBufferSubData(Renderer->Buffers[renderer_vbo_Colors], 
                                     CurrentInstanceIndex * sizeof(c4f),
                                     sizeof(c4f), 
                                     &Data->Colors);
                
                glNamedBufferSubData(Renderer->Buffers[renderer_vbo_Texture],
                                     CurrentInstanceIndex * sizeof(quad2f),
                                     sizeof(quad2f),
                                     &Data->TextureCoords);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f Transform = Transpose(Data->Transform);
                glNamedBufferSubData(Renderer->Buffers[renderer_vbo_Transform], 
                                     CurrentInstanceIndex* sizeof(m44f), 
                                     sizeof(m44f), 
                                     &Transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++InstancesToDrawCount;
                ++CurrentInstanceIndex;
                
            } break;
        }
    }
    
    if (InstancesToDrawCount != 0) {
        glBindTexture(GL_TEXTURE_2D, CurrentTexture);
        glBindVertexArray(Renderer->Blueprint);
        glUseProgram(Renderer->Shader);
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                            6, 
                                            GL_UNSIGNED_BYTE, 
                                            nullptr, 
                                            InstancesToDrawCount,
                                            LastDrawnInstanceIndex);
    }
}
#endif
