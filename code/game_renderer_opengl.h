#ifndef __GAME_RENDERER_OPENGL__
#define __GAME_RENDERER_OPENGL__

#include "game_renderer.h"
#include "game_renderer_opengl_shaders.inl"

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
    // TODO(Momo): Make this dynamic? 
    GLuint GameToRendererTextureTable[0xFF] = {};
    GLuint WindowWidth, WindowHeight;
    GLuint RenderWidth, RenderHeight;
};

static inline void 
GlAttachShader(GLuint program, GLenum type, const GLchar* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

static inline void AlignViewport(renderer_opengl* Renderer) {
    auto Region = GetRenderRegion(Renderer->WindowWidth, Renderer->WindowHeight, Renderer->RenderWidth, Renderer->RenderHeight);
    
    GLuint x, y, w, h;
    x = Region.Min.X;
    y = Region.Min.Y;
    w = GetWidth(Region);
    h = GetHeight(Region);
    
    glViewport(x, y, w, h);
    glScissor(x, y, w, h);
}


static inline void 
SetRenderResolution(renderer_opengl* Renderer, GLuint RenderWidth, GLuint RenderHeight){
    Renderer->RenderWidth = RenderWidth;
    Renderer->RenderHeight = RenderHeight;
    AlignViewport(Renderer);
}

static inline void 
SetWindowResolution(renderer_opengl* Renderer, GLuint WindowWidth, GLuint WindowHeight) {
    Renderer->WindowWidth = WindowWidth;
    Renderer->WindowHeight = WindowHeight;
    AlignViewport(Renderer);
}

static inline bool
Init(renderer_opengl* Renderer, GLuint WindowWidth, GLuint WindowHeight, GLsizei MaxEntities) 
{
    Renderer->MaxEntities = MaxEntities;
    Renderer->RenderWidth = WindowWidth;
    Renderer->RenderHeight = WindowHeight;
    Renderer->WindowWidth = WindowWidth;
    Renderer->WindowHeight = WindowHeight;
    
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
#if INTERNAL
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    
    AlignViewport(Renderer);
    
    
    // NOTE(Momo): Setup VBO
    glCreateBuffers(renderer_vbo_Max, Renderer->Buffers);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Model], sizeof(QuadModel), QuadModel, 0);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Indices], sizeof(QuadIndices), QuadIndices, 0);
    
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Texture], sizeof(v2f) * 4 * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Colors], sizeof(v4f) * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glNamedBufferStorage(Renderer->Buffers[renderer_vbo_Transform], sizeof(m44f) * MaxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    glCreateVertexArrays(1, &Renderer->Blueprint);
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Model, Renderer->Buffers[renderer_vbo_Model], 0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Texture, Renderer->Buffers[renderer_vbo_Texture], 0, sizeof(f32) * 8);
    glVertexArrayVertexBuffer(Renderer->Blueprint, renderer_vaobind_Colors, Renderer->Buffers[renderer_vbo_Colors],  0, sizeof(v4f));
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
    
    
    // NOTE(Momo): Setup Shader Program
    Renderer->Shader = glCreateProgram();
    GlAttachShader(Renderer->Shader, GL_VERTEX_SHADER, RendererOpenGLVertexShader);
    GlAttachShader(Renderer->Shader, GL_FRAGMENT_SHADER, RendererOpenGLVFragmentShader);
    glLinkProgram(Renderer->Shader);
    GLint result;
    glGetProgramiv(Renderer->Shader, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[Kilobyte];
        glGetProgramInfoLog(Renderer->Shader, Kilobyte, nullptr, msg);
        // TODO(Momo): Log?
        return false;
    }
    
    // NOTE(Momo): Blank texture setup
    struct { u8 E[4]; } Pixel = {255, 255, 255, 255};
    glCreateTextures(GL_TEXTURE_2D, 1, &Renderer->BlankTexture);
    glTextureStorage2D(Renderer->BlankTexture, 1, GL_RGBA8, 1, 1);
    glTextureSubImage2D(Renderer->BlankTexture, 
                        0, 0, 0, 
                        1, 1, 
                        GL_RGBA, GL_UNSIGNED_BYTE, 
                        &Pixel);
    
    return true;
}

static inline void 
DrawInstances(renderer_opengl* Renderer, GLint Texture, u32 InstancesToDraw, u32 IndexToDrawFrom) {
    if (InstancesToDraw > 0) {
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBindVertexArray(Renderer->Blueprint);
        glUseProgram(Renderer->Shader);
        
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                            6, 
                                            GL_UNSIGNED_BYTE, 
                                            nullptr, 
                                            InstancesToDraw,
                                            IndexToDrawFrom);
    }
}


static inline void
Render(renderer_opengl* Renderer, commands* Commands) 
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
            case render_command_set_design_resolution::TypeId: {
                using data_t = render_command_set_design_resolution;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                SetRenderResolution(Renderer, Data->Width, Data->Height);
            } break;
            case render_command_set_basis::TypeId: {
                using data_t = render_command_set_basis;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                
                DrawInstances(Renderer, CurrentTexture, InstancesToDrawCount, LastDrawnInstanceIndex);
                LastDrawnInstanceIndex += InstancesToDrawCount;
                InstancesToDrawCount = 0;
                
                auto Result = Transpose(Data->Basis);
                GLint uProjectionLoc = glGetUniformLocation(Renderer->Shader, "uProjection");
                glProgramUniformMatrix4fv(Renderer->Shader, uProjectionLoc, 1, GL_FALSE, Result[0]);
                
            } break;
            case render_command_link_texture::TypeId: {
                using data_t = render_command_link_texture;
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
            case render_command_clear_color::TypeId: {
                using data_t = render_command_clear_color;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                glClearColor(Data->Colors.R, Data->Colors.G, Data->Colors.B, Data->Colors.A);
            } break;
            case render_command_draw_quad::TypeId: {
                using data_t = render_command_draw_quad;
                auto* Data = (data_t*)GetDataFromEntry(Commands, Entry);
                
                // If the game texture handle does not exist in the lookup table, add texture to renderer and register it into the lookup table
                GLuint RendererTextureHandle = Renderer->BlankTexture;
                
                if (RendererTextureHandle == 0) {
                    // TODO(Momo): Maybe render a dummy texture?
                    Assert(false);
                }
                
                // NOTE(Momo): If the currently set texture is not same as the currently processed texture, batch draw all instances before the current instance.
                if (CurrentTexture != RendererTextureHandle) {
                    DrawInstances(Renderer, CurrentTexture, InstancesToDrawCount, LastDrawnInstanceIndex);
                    LastDrawnInstanceIndex += InstancesToDrawCount;
                    InstancesToDrawCount = 0;
                    CurrentTexture = RendererTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                glNamedBufferSubData(Renderer->Buffers[renderer_vbo_Colors], 
                                     CurrentInstanceIndex * sizeof(v4f),
                                     sizeof(v4f), 
                                     &Data->Colors);
                
                glNamedBufferSubData(Renderer->Buffers[renderer_vbo_Texture],
                                     CurrentInstanceIndex * sizeof(quad2f),
                                     sizeof(quad2f),
                                     &StandardQuadUV);
                
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
            case render_command_draw_textured_quad::TypeId: {
                using data_t = render_command_draw_textured_quad;
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
                    DrawInstances(Renderer, CurrentTexture, InstancesToDrawCount, LastDrawnInstanceIndex);
                    LastDrawnInstanceIndex += InstancesToDrawCount;
                    InstancesToDrawCount = 0;
                    CurrentTexture = RendererTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                glNamedBufferSubData(Renderer->Buffers[renderer_vbo_Colors], 
                                     CurrentInstanceIndex * sizeof(v4f),
                                     sizeof(v4f), 
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
    
    DrawInstances(Renderer, CurrentTexture, InstancesToDrawCount, LastDrawnInstanceIndex);
    
}



#endif
