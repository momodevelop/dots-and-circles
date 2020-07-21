#ifndef __PLATFORM_RENDERER_OPENGL__
#define __PLATFORM_RENDERER_OPENGL__

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
    
    // NOTE(Momo): A table mapping 'game texture handler' <-> 'opengl texture handler'  
    GLuint GameToRendererTextureTable[1024] = {};
    
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
    constexpr static const char* vertexShader = "#version 450 core\nlayout(location=0) in vec3 aModelVtx; \nlayout(location=1) in vec4 aColor;\nlayout(location=2) in vec2 aTexCoord;\nlayout(location=3) in mat4 aTransform;\nout vec4 mColor;\nout vec2 mTexCoord;\nuniform mat4 uProjection;\n\n\
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            void main(void) {\ngl_Position = uProjection * aTransform *  vec4(aModelVtx, 1.0);\nmColor = aColor;\n\
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            mTexCoord = aTexCoord;\n}";
    
    constexpr static const char* fragmentShader = "#version 450 core\nout vec4 fragColor;\nin vec4 mColor;\nin vec2 mTexCoord;\nuniform sampler2D uTexture;\nvoid main(void) {\nfragColor = texture(uTexture, mTexCoord) * mColor; \n}";
    
    
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
    glProgramUniformMatrix4fv(Renderer->Shader, uProjectionLoc, 1, GL_FALSE, uProjection[0]);
    
    return 0;
}




static inline void
Render(renderer_opengl* Renderer, render_commands* Commands) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArray(Renderer->Blueprint);
    glUseProgram(Renderer->Shader);
    
    // TODO(Momo): Better way to do this without binding texture first?
    GLuint CurrentTexture = 0;
    u32 InstancesToDrawCount = 0;
    u32 LastDrawnInstanceIndex = 0;
    u32 CurrentInstanceIndex = 0;
    
    using HeaderType = render_command_header;
    for (render_command_header* Itr = (HeaderType*)Commands->Head; 
         Itr != nullptr; 
         Itr = Itr->Next) {
        
        render_command_header* CurrentHeader = (HeaderType*)Itr;
        switch(CurrentHeader->Type) {
            case render_command_entry_clear::TypeId: {
                using EntryType = render_command_entry_clear;
                auto Entry = (EntryType*)CurrentHeader->Entry;
                glClearColor(Entry->Colors.Red, Entry->Colors.Green, Entry->Colors.Blue, Entry->Colors.Alpha);
            } break;
            case render_command_entry_textured_quad::TypeId: {
                // Get the entry
                using EntryType = render_command_entry_textured_quad;
                auto Entry = (EntryType*)CurrentHeader->Entry;
                
                // If the game texture handle does not exist in the lookup table, add texture to renderer and register it into the lookup table
                u32 GameTextureHandle = Entry->Texture.Handle;
                GLuint RendererTextureHandle = Renderer->GameToRendererTextureTable[GameTextureHandle];
                
                // TODO(Momo): Abstract away the texture loading to a function?
                // NOTE(Momo): Renderer Texture Handle is invalid
                if (RendererTextureHandle == 0) {
                    // NOTE(Momo): Add texture to opengl
                    GLuint* const TextureTableEntry = &Renderer->GameToRendererTextureTable[GameTextureHandle];
                    glCreateTextures(GL_TEXTURE_2D, 1, TextureTableEntry);
                    glTextureStorage2D((*TextureTableEntry), 1, GL_RGBA8, Entry->Texture.Bitmap.Width, Entry->Texture.Bitmap.Height);
                    glTextureSubImage2D((*TextureTableEntry), 
                                        0, 0, 0, 
                                        Entry->Texture.Bitmap.Width, Entry->Texture.Bitmap.Height, 
                                        GL_RGBA, GL_UNSIGNED_BYTE, 
                                        Entry->Texture.Bitmap.Pixels);
                }
                RendererTextureHandle = Renderer->GameToRendererTextureTable[GameTextureHandle];
                
                // NOTE(Momo): If the currently set texture is not same as the currently processed texture, 
                // batch draw all instances before the current instance.
                if (CurrentTexture != RendererTextureHandle) {
                    if (InstancesToDrawCount != 0) {
                        glBindTexture(GL_TEXTURE_2D, CurrentTexture);
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
                glNamedBufferSubData(Renderer->Buffers[Vbo_Colors], 
                                     CurrentInstanceIndex * sizeof(c4f),
                                     sizeof(c4f), 
                                     &Entry->Colors);
                
                m44f Transform = Transpose(Entry->Transform);
                glNamedBufferSubData(Renderer->Buffers[Vbo_Transform], 
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
        glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                            6, 
                                            GL_UNSIGNED_BYTE, 
                                            nullptr, 
                                            InstancesToDrawCount,
                                            LastDrawnInstanceIndex);
    } 
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
