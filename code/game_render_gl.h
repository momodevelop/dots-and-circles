#ifndef __PLATFORM_SDL_RENDERER_GL__
#define __PLATFORM_SDL_RENDERER_GL__

// TODO(Momo): Should this be game specific opengl-code...?


// NOTE(Momo): Very simple 2D renderer
// We will only have 1 model (a rectangle), which will have the 
// capabilities have having 1 texture, 1 color, and 1 transform.

#include "ryoji_maths.h"



// NOTE(Momo): VBO types
enum {
    GlRendererVbo_Model,
    GlRendererVbo_Indices,
    GlRendererVbo_Colors,
    GlRendererVbo_Texture,
    GlRendererVbo_Transform,
    GlRendererVbo_Max
};

// NOTE(Momo): Attributes
enum {
    GlRendererAtb_Model,
    GlRendererAtb_Colors,
    GlRendererAtb_Texture, 
    
    // NOTE(Momo): Because it's matrix4x4, so need 4x vec4's
    GlRendererAtb_Transform1, 
    GlRendererAtb_Transform2,
    GlRendererAtb_Transform3,
    GlRendererAtb_Transform4
};

// NOTE(Momo): VAO bindings
enum {
    GlRendererVaoBind_Model,
    GlRendererVaoBind_Colors,
    GlRendererVaoBind_Texture,
    GlRendererVaoBind_Transform,
};

// NOTE(Momo): This is a singletion? lol!
struct {
    GLuint Buffers[GlRendererVbo_Max]; 
    GLuint Shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint Blueprint; 
    GLsizei MaxEntities;
    
    // TODO(Momo): Proper texture management
    GLuint texture;
    
    f32 ViewportWidth, ViewportHeight;
} gGlRenderer = {};


static inline void 
GlAttachShader(GLuint program, GLenum type, const GLchar* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

static inline bool
GlRendererInit(f32 viewportWidth, f32 viewportHeight, GLsizei maxEntities, loaded_bitmap* Bmp) {
    gGlRenderer.ViewportWidth = viewportWidth;
    gGlRenderer.ViewportHeight = viewportHeight;
    gGlRenderer.MaxEntities = maxEntities;
    
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
    
    
    // NOTE(Momo): Setup VBO
    glCreateBuffers(GlRendererVbo_Max, gGlRenderer.Buffers);
    glNamedBufferStorage(gGlRenderer.Buffers[GlRendererVbo_Model], sizeof(quadModel), quadModel, 0);
    glNamedBufferStorage(gGlRenderer.Buffers[GlRendererVbo_Indices], sizeof(quadIndices), quadIndices, 0);
    glNamedBufferStorage(gGlRenderer.Buffers[GlRendererVbo_Texture], sizeof(quadTexCoords), quadTexCoords, 0);
    
    
    // NOTE(Momo): colors are 4x vec4, one for each side
    glNamedBufferStorage(gGlRenderer.Buffers[GlRendererVbo_Colors], sizeof(f32)*16 * maxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    glNamedBufferStorage(gGlRenderer.Buffers[GlRendererVbo_Transform], sizeof(f32)*16 * maxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    glCreateVertexArrays(1, &gGlRenderer.Blueprint);
    glVertexArrayVertexBuffer(gGlRenderer.Blueprint, GlRendererVaoBind_Model, gGlRenderer.Buffers[GlRendererVbo_Model],   0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(gGlRenderer.Blueprint, GlRendererVaoBind_Texture, gGlRenderer.Buffers[GlRendererVbo_Texture], 0, sizeof(f32)*2);
    glVertexArrayVertexBuffer(gGlRenderer.Blueprint, GlRendererVaoBind_Colors, gGlRenderer.Buffers[GlRendererVbo_Colors],  0, sizeof(f32)*4);
    glVertexArrayVertexBuffer(gGlRenderer.Blueprint, GlRendererVaoBind_Transform, gGlRenderer.Buffers[GlRendererVbo_Transform], 0, sizeof(m44f));
    
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Model); 
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Model, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Model, GlRendererVaoBind_Model);
    
    // aColor
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Colors); 
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Colors, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Colors, GlRendererVaoBind_Colors);
    
    // NOTE(Momo): If binding divisor is zero, advance once per vertex
    glVertexArrayBindingDivisor(gGlRenderer.Blueprint, GlRendererVaoBind_Colors, 0); 
    
    // aTexCoord
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Texture); 
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Texture, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Texture, GlRendererVaoBind_Texture);
    
    
    // aTransform
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Transform1); 
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Transform1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 0 * 4);
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Transform2);
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Transform2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Transform3); 
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Transform3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(gGlRenderer.Blueprint, GlRendererAtb_Transform4); 
    glVertexArrayAttribFormat(gGlRenderer.Blueprint, GlRendererAtb_Transform4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Transform1, GlRendererVaoBind_Transform);
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Transform2, GlRendererVaoBind_Transform);
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Transform3, GlRendererVaoBind_Transform);
    glVertexArrayAttribBinding(gGlRenderer.Blueprint, GlRendererAtb_Transform4, GlRendererVaoBind_Transform);
    
    // NOTE(Momo): If binding divisor is N, advance N per instance
    glVertexArrayBindingDivisor(gGlRenderer.Blueprint, GlRendererVaoBind_Transform, 1); 
    
    
    // NOTE(Momo): Setup indices
    glVertexArrayElementBuffer(gGlRenderer.Blueprint, gGlRenderer.Buffers[GlRendererVbo_Indices]);
    
    // Setup Shader Program
    gGlRenderer.Shader = glCreateProgram();
    GlAttachShader(gGlRenderer.Shader, GL_VERTEX_SHADER, vertexShader);
    GlAttachShader(gGlRenderer.Shader, GL_FRAGMENT_SHADER, fragmentShader);
    glLinkProgram(gGlRenderer.Shader);
    GLint result;
    glGetProgramiv(gGlRenderer.Shader, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[KILOBYTE];
        glGetProgramInfoLog(gGlRenderer.Shader, KILOBYTE, nullptr, msg);
        SDL_Log("Linking program failed:\n %s\n", msg);
        return false;
    }
    
    // NOTE(Momo): Setup Orthographic Projection
    GLint uProjectionLoc = glGetUniformLocation(gGlRenderer.Shader, "uProjection");
    auto uProjection  = MakeOrthographicMatrix(-1.f, 1.f,
                                               -1.f, 1.f,
                                               -1.f, 1.f,
                                               -(gGlRenderer.ViewportWidth) * 0.5f,  
                                               (gGlRenderer.ViewportWidth) * 0.5f, 
                                               -(gGlRenderer.ViewportHeight) * 0.5f, 
                                               (gGlRenderer.ViewportHeight) * 0.5f,
                                               -100.f, 100.f,
                                               true);
    uProjection = Transpose(uProjection);
    glProgramUniformMatrix4fv(gGlRenderer.Shader, uProjectionLoc, 1, GL_FALSE, *uProjection.Arr);
    
    
    // TODO(Momo): Do proper texture management
    glCreateTextures(GL_TEXTURE_2D, 1, &gGlRenderer.texture);
    glTextureStorage2D(gGlRenderer.texture, 1, GL_RGBA8, Bmp->Width, Bmp->Height);
    
    glTextureSubImage2D(gGlRenderer.texture, 0, 0, 0, Bmp->Width, Bmp->Height, GL_RGBA, GL_UNSIGNED_BYTE, Bmp->Pixels);
    
    Bmp->Pixels = nullptr;
    
    return 0;
}


static inline void 
GlProcessRenderGroup(render_group * RenderGroup) {
    // TODO(Momo):  shift this away to link up with game code
    
    for (int i = 0; i < RenderGroup->Count; ++i) {
        glNamedBufferSubData(gGlRenderer.Buffers[GlRendererVbo_Colors], i * sizeof(m44f), sizeof(m44f), &RenderGroup->Colors[i]);
        glNamedBufferSubData(gGlRenderer.Buffers[GlRendererVbo_Transform], i * sizeof(m44f), sizeof(m44f), &RenderGroup->Transforms[i]);
        
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gGlRenderer.Shader);
    glBindTexture(GL_TEXTURE_2D, gGlRenderer.texture);
    glBindVertexArray(gGlRenderer.Blueprint);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, gGlRenderer.MaxEntities);
    
}



















#endif
