#ifndef __PLATFORM_SDL_RENDERER_GL__
#define __PLATFORM_SDL_RENDERER_GL__

// NOTE(Momo): Very simple 2D renderer
// We will only have 1 model (a rectangle), which will have the 
// capabilities have having 1 texture, 1 color, and 1 transform.

#include "ryoji_maths.h"



// NOTE(Momo): VBO types
enum {
    VBO_MODEL,
    VBO_INDICES,
    VBO_COLORS,
    VBO_TEXTURE,
    VBO_TRANSFORM,
    VBO_MAX
};

// NOTE(Momo): Attributes
enum {
    ATB_MODEL,
    ATB_COLORS,
    ATB_TEXTURE, 
    
    // NOTE(Momo): Because it's matrix4x4, so need 4x vec4's
    ATB_TRANSFORM1, 
    ATB_TRANSFORM2,
    ATB_TRANSFORM3,
    ATB_TRANSFORM4
};

// NOTE(Momo): VAO bindings
enum {
    VAO_MODEL_BIND,
    VAO_COLORS_BIND,
    VAO_TEXTURE_BIND,
    VAO_TRANSFORM_BIND,
};

struct gl_renderer {
    GLuint Buffers[VBO_MAX]; 
    GLuint Shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint Blueprint; 
    
    GLsizei MaxEntities;
    
    // TODO(Momo): Proper texture management
    GLuint texture;
    
    f32 ViewportWidth, ViewportHeight;
};


static inline void 
GLAttachShader(GLuint program, GLenum type, const GLchar* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glDeleteShader(shader);
}


static inline bool
Init(gl_renderer* r, f32 viewportWidth, f32 viewportHeight, GLsizei maxEntities, Bmp* bmp) {
    r->ViewportWidth = viewportWidth;
    r->ViewportHeight = viewportHeight;
    
    
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
    
    f32 quadColorful[] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.0f,
        1.f, 1.f, 1.f, 0.0f,
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
    glCreateBuffers(VBO_MAX, r->Buffers);
    glNamedBufferStorage(r->Buffers[VBO_MODEL], sizeof(quadModel), quadModel, 0);
    glNamedBufferStorage(r->Buffers[VBO_INDICES], sizeof(quadIndices), quadIndices, 0);
    glNamedBufferStorage(r->Buffers[VBO_TEXTURE], sizeof(quadTexCoords), quadTexCoords, 0);
    glNamedBufferStorage(r->Buffers[VBO_COLORS], sizeof(quadColorful), quadColorful, 0);
    
    // NOTE(Momo): colors are 4x vec4, one for each side
    //glNamedBufferStorage(r->Buffers[VBO_COLORS], sizeof(f32)*16 * maxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    glNamedBufferStorage(r->Buffers[VBO_TRANSFORM], sizeof(f32)*16 * maxEntities, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    glCreateVertexArrays(1, &r->Blueprint);
    glVertexArrayVertexBuffer(r->Blueprint, VAO_MODEL_BIND, r->Buffers[VBO_MODEL],   0, sizeof(f32)*3);
    glVertexArrayVertexBuffer(r->Blueprint, VAO_TEXTURE_BIND, r->Buffers[VBO_TEXTURE], 0, sizeof(f32)*2);
    glVertexArrayVertexBuffer(r->Blueprint, VAO_COLORS_BIND, r->Buffers[VBO_COLORS],  0, sizeof(f32)*4);
    glVertexArrayVertexBuffer(r->Blueprint, VAO_TRANSFORM_BIND, r->Buffers[VBO_TRANSFORM], 0, sizeof(m44f));
    
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    glEnableVertexArrayAttrib(r->Blueprint, ATB_MODEL); 
    glVertexArrayAttribFormat(r->Blueprint, ATB_MODEL, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r->Blueprint, ATB_MODEL, VAO_MODEL_BIND);
    
    // aColor
    glEnableVertexArrayAttrib(r->Blueprint, ATB_COLORS); 
    glVertexArrayAttribFormat(r->Blueprint, ATB_COLORS, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r->Blueprint, ATB_COLORS, VAO_COLORS_BIND);
    //glVertexArrayBindingDivisor(r->Blueprint, VAO_COLORS_BIND, 1); 
    
    
    // aTexCoord
    glEnableVertexArrayAttrib(r->Blueprint, ATB_TEXTURE); 
    glVertexArrayAttribFormat(r->Blueprint, ATB_TEXTURE, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(r->Blueprint, ATB_TEXTURE, VAO_TEXTURE_BIND);
    
    
    // aTransform
    glEnableVertexArrayAttrib(r->Blueprint, ATB_TRANSFORM1); 
    glVertexArrayAttribFormat(r->Blueprint, ATB_TRANSFORM1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 0 * 4);
    glEnableVertexArrayAttrib(r->Blueprint, ATB_TRANSFORM2);
    glVertexArrayAttribFormat(r->Blueprint, ATB_TRANSFORM2, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 1 * 4);
    glEnableVertexArrayAttrib(r->Blueprint, ATB_TRANSFORM3); 
    glVertexArrayAttribFormat(r->Blueprint, ATB_TRANSFORM3, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 2 * 4);
    glEnableVertexArrayAttrib(r->Blueprint, ATB_TRANSFORM4); 
    glVertexArrayAttribFormat(r->Blueprint, ATB_TRANSFORM4, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 3 * 4);
    
    glVertexArrayAttribBinding(r->Blueprint, ATB_TRANSFORM1, VAO_TRANSFORM_BIND);
    glVertexArrayAttribBinding(r->Blueprint, ATB_TRANSFORM2, VAO_TRANSFORM_BIND);
    glVertexArrayAttribBinding(r->Blueprint, ATB_TRANSFORM3, VAO_TRANSFORM_BIND);
    glVertexArrayAttribBinding(r->Blueprint, ATB_TRANSFORM4, VAO_TRANSFORM_BIND);
    glVertexArrayBindingDivisor(r->Blueprint, VAO_TRANSFORM_BIND, 1); 
    
    
    // NOTE(Momo): Setup indices
    glVertexArrayElementBuffer(r->Blueprint, r->Buffers[VBO_INDICES]);
    
    // Setup Shader Program
    r->Shader = glCreateProgram();
    GLAttachShader(r->Shader, GL_VERTEX_SHADER, vertexShader);
    GLAttachShader(r->Shader, GL_FRAGMENT_SHADER, fragmentShader);
    glLinkProgram(r->Shader);
    GLint result;
    glGetProgramiv(r->Shader, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        char msg[KILOBYTE];
        glGetProgramInfoLog(r->Shader, KILOBYTE, nullptr, msg);
        SDL_Log("Linking program failed:\n %s\n", msg);
        return false;
    }
    
    // NOTE(Momo): Setup Orthographic Projection
    GLint uProjectionLoc = glGetUniformLocation(r->Shader, "uProjection");
    auto uProjection  = Orthographic(-1.f, 1.f,
                                     -1.f, 1.f,
                                     -1.f, 1.f,
                                     -(r->ViewportWidth) * 0.5f,  
                                     (r->ViewportWidth) * 0.5f, 
                                     -(r->ViewportHeight) * 0.5f, 
                                     (r->ViewportHeight) * 0.5f,
                                     -100.f, 100.f,
                                     true);
    uProjection = Transpose(uProjection);
    glProgramUniformMatrix4fv(r->Shader, uProjectionLoc, 1, GL_FALSE, *uProjection.Arr);
    
    
    // TODO(Momo): Do proper texture management
    glCreateTextures(GL_TEXTURE_2D, 1, &r->texture);
    glTextureStorage2D(r->texture, 1, GL_RGBA8, bmp->InfoHeader.Width, bmp->InfoHeader.Height);
    
    glTextureSubImage2D(r->texture, 0, 0, 0, bmp->InfoHeader.Width, bmp->InfoHeader.Height, GL_RGBA, GL_UNSIGNED_BYTE, 
                        bmp->Pixels);
    
    return 0;
}

static inline void 
Render(gl_renderer* r) {
    // TODO(Momo):  shift this away to link up with game code
    m44f instanceTransforms[1024] = {};
    f32 startX = -r->ViewportWidth/2.f;
    f32 startY = -r->ViewportHeight/2.f;
    f32 xOffset = 200.f;
    f32 yOffset = 200.f;
    f32 currentXOffset = 0.f;
    f32 currentYOffset = 0.f;
    f32 quadColorful[] = {
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.0f,
        1.f, 1.f, 1.f, 0.0f,
    };
    
    
    for (int i = 0; i < 1024; ++i) {
        instanceTransforms[i] = 
            Translation(startX + currentXOffset, startY + currentYOffset, 0.f) *
            Scale(100.f, 100.f, 1.f);
        instanceTransforms[i] = Transpose(instanceTransforms[i]);
        
        //glNamedBufferSubData(r->Buffers[VBO_COLORS], i * sizeof(m44f), sizeof(m44f), quadColorful);
        
        glNamedBufferSubData(r->Buffers[VBO_TRANSFORM], i * sizeof(m44f), sizeof(m44f), &instanceTransforms[i]);
        
        currentXOffset += xOffset;
        if (currentXOffset > r->ViewportWidth) {
            currentXOffset = 0.f;
            currentYOffset += yOffset;
        }
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(r->Shader);
    glBindTexture(GL_TEXTURE_2D, r->texture);
    glBindVertexArray(r->Blueprint);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, r->MaxEntities);
    
}



















#endif