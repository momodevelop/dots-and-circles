#ifndef __RENDERER_OPENGL__
#define __RENDERER_OPENGL__

#include "game_renderer.h"

// Configuration
#define Opengl_Max_Textures 8
#define Opengl_Max_Entities 4096

// Opengl typedefs
#define GL_TRUE                 1
#define GL_FALSE                0

#define GL_DEPTH_TEST                   0x0B71
#define GL_SCISSOR_TEST                 0x0C11
#define GL_DEPTH_BUFFER_BIT             0x00000100
#define GL_C4f_BUFFER_BIT             0x00004000
#define GL_DEBUG_OUTPUT                 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS     0x8242
#define GL_FLOAT                        0x1406
#define GL_DYNAMIC_STORAGE_BIT          0x0100
#define GL_TEXTURE_2D                   0x0DE1
#define GL_FRAGMENT_SHADER              0x8B30
#define GL_VERTEX_SHADER                0x8B31
#define GL_LINK_STATUS                  0x8B82
#define GL_BLEND                        0x0BE2
#define GL_SRC_ALPHA                    0x0302
#define GL_ONE_MINUS_SRC_ALPHA          0x0303
#define GL_RGBA                         0x1908
#define GL_RGBA8                        0x8058
#define GL_UNSIGNED_BYTE                0x1401
#define GL_TRIANGLES                    0x0004
#define GL_NEAREST                      0x2600
#define GL_LINEAR                       0x2601
#define GL_TEXTURE_MIN_FILTER           0x2801
#define GL_TEXTURE_MAG_FILTER           0x2800
#define GL_DEBUG_SOURCE_API 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY 0x8249
#define GL_DEBUG_SOURCE_APPLICATION 0x824A
#define GL_DEBUG_SOURCE_OTHER 0x824B
#define GL_DEBUG_SEVERITY_HIGH 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM 0x9147
#define GL_DEBUG_SEVERITY_LOW 0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B
#define GL_DEBUG_TYPE_ERROR 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE 0x8250
#define GL_DEBUG_TYPE_OTHER 0x8251
#define GL_DEBUG_TYPE_MARKER 0x8268

typedef s32  GLenum;
typedef s32  GLint; 
typedef s32  GLsizei;
typedef u32  GLuint;
typedef c8   GLchar;
typedef u32  GLbitfield;
typedef f32  GLclampf;
typedef iptr GLsizeiptr; 
typedef iptr GLintptr;
typedef b8   GLboolean;
typedef f32  GLfloat;

#define OpenglFunction(Name) opengl_func_##Name
#define OpenglFunctionPtr(Name) OpenglFunction(Name)* Name
#define OpenglDebugCallbackFunc(Name) void Name(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *msg,const void *userParam)
typedef OpenglDebugCallbackFunc(GLDEBUGPROC);

typedef void    OpenglFunction(glEnable)(GLenum cap);
typedef void    OpenglFunction(glDisable)(GLenum cap);
typedef void    OpenglFunction(glViewport)(GLint x, 
                                           GLint y, 
                                           GLsizei width, 
                                           GLsizei height);
typedef void    OpenglFunction(glScissor)(GLint x, 
                                          GLint y, 
                                          GLsizei width, 
                                          GLsizei height); 
typedef GLuint  OpenglFunction(glCreateShader)(GLenum type);
typedef void    OpenglFunction(glCompileShader)(GLuint program);
typedef void    OpenglFunction(glShaderSource)(GLuint shader, 
                                               GLsizei count, 
                                               GLchar** string, 
                                               GLint* length);
typedef void    OpenglFunction(glAttachShader)(GLuint program, GLuint shader);
typedef void    OpenglFunction(glDeleteShader)(GLuint program);
typedef void    OpenglFunction(glClear)(GLbitfield mask);
typedef void    OpenglFunction(glClearColor)(GLclampf r, 
                                             GLclampf g, 
                                             GLclampf b, 
                                             GLclampf a);
typedef void    OpenglFunction(glCreateBuffers)(GLsizei n, GLuint* buffers);
typedef void    OpenglFunction(glNamedBufferStorage)(GLuint buffer, 
                                                     GLsizeiptr size, 
                                                     const void* data, 
                                                     GLbitfield flags);
typedef void    OpenglFunction(glCreateVertexArrays)(GLsizei n, GLuint* arrays);
typedef void    OpenglFunction(glVertexArrayVertexBuffer)(GLuint vaobj, 
                                                          GLuint bindingindex, 
                                                          GLuint buffer, 
                                                          GLintptr offset, 
                                                          GLsizei stride);
typedef void    OpenglFunction(glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
typedef void    OpenglFunction(glVertexArrayAttribFormat)(GLuint vaobj,
                                                          GLuint attribindex,
                                                          GLint size,
                                                          GLenum type,
                                                          GLboolean normalized,
                                                          GLuint relativeoffset);
typedef void    OpenglFunction(glVertexArrayAttribBinding)(GLuint vaobj,
                                                           GLuint attribindex,
                                                           GLuint bindingindex);
typedef void    OpenglFunction(glVertexArrayBindingDivisor)(GLuint vaobj,
                                                            GLuint bindingindex,
                                                            GLuint divisor);
typedef void    OpenglFunction(glBlendFunc)(GLenum sfactor, GLenum dfactor);
typedef void    OpenglFunction(glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
typedef GLuint  OpenglFunction(glCreateProgram)();
typedef void    OpenglFunction(glLinkProgram)(GLuint program);
typedef void    OpenglFunction(glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
typedef void    OpenglFunction(glGetProgramInfoLog)(GLuint program, 
                                                    GLsizei maxLength,
                                                    GLsizei* length,
                                                    GLchar* infoLog);
typedef void    OpenglFunction(glCreateTextures)(GLenum target, 
                                                 GLsizei n, 
                                                 GLuint* textures);
typedef void    OpenglFunction(glTextureStorage2D)(GLuint texture,
                                                   GLsizei levels,
                                                   GLenum internalformat,
                                                   GLsizei width,
                                                   GLsizei height);
typedef void    OpenglFunction(glTextureSubImage2D)(GLuint texture,
                                                    GLint level,
                                                    GLint xoffset,
                                                    GLint yoffset,
                                                    GLsizei width,
                                                    GLsizei height,
                                                    GLenum format,
                                                    GLenum type,
                                                    const void* pixels);
typedef void    OpenglFunction(glBindTexture)(GLenum target, GLuint texture);
typedef void    OpenglFunction(glTexParameteri)(GLenum target ,GLenum pname, GLint param);
typedef void    OpenglFunction(glBindVertexArray)(GLuint array);
typedef void    OpenglFunction(glDrawElementsInstancedBaseInstance)(GLenum mode,
                                                                    GLsizei count,
                                                                    GLenum type,
                                                                    const void* indices,
                                                                    GLsizei instancecount,
                                                                    GLuint baseinstance);
typedef void    OpenglFunction(glUseProgram)(GLuint program);
typedef void    OpenglFunction(glNamedBufferSubData)(GLuint buffer,
                                                     GLintptr offset,
                                                     GLsizeiptr size,
                                                     const void* data);
typedef GLint   OpenglFunction(glGetUniformLocation)(GLuint program,
                                                     const GLchar* name);
typedef void    OpenglFunction(glProgramUniformMatrix4fv)(GLuint program,
                                                          GLint location,
                                                          GLsizei count,
                                                          GLboolean transpose,
                                                          const GLfloat* value);
typedef void    OpenglFunction(glUseProgram)(GLuint program);
typedef void    OpenglFunction(glDeleteTextures)(GLsizei n, 
                                                 const GLuint* textures);
typedef void    OpenglFunction(glDebugMessageCallbackARB)(GLDEBUGPROC *callback, 
                                                          const void *userParam);


// Stuff to work with game
constexpr static inline f32 QuadModel[] = {
    -0.5f, -0.5f, 0.0f,  // bottom left
    0.5f, -0.5f, 0.0f,  // bottom right
    0.5f,  0.5f, 0.0f,  // top right
    -0.5f,  0.5f, 0.0f,   // top left 
};

constexpr static inline u8 QuadIndices[] = {
    0, 1, 2,
    0, 2, 3,
};

constexpr static inline f32 QuadUV[] = {
    0.0f, 1.0f,  // top left
    1.0f, 1.0f, // top right
    1.0f, 0.f, // bottom right
    0.f, 0.f, // bottom left
};

// NOTE(Momo): Buffers
enum {
    OpenglVbo_Model,
    OpenglVbo_Indices,
    OpenglVbo_Colors,
    OpenglVbo_Texture,
    OpenglVbo_Transform,
    OpenglVbo_Max
};

constexpr static const char* OpenglVertexShader = R"###(
#version 450 core
layout(location=0) in vec3 aModelVtx; 
layout(location=1) in vec4 aColor;
layout(location=2) in vec2 aTexCoord[4];
layout(location=6) in mat4 aTransform;
out vec4 mColor;
out vec2 mTexCoord;
uniform mat4 uProjection;

void main(void) {
	gl_Position = uProjection * aTransform *  vec4(aModelVtx, 1.0);
	mColor = aColor;
	mTexCoord = aTexCoord[gl_VertexID];
	//mTexCoord.y = 1.0 - mTexCoord.y;
})###";

constexpr static const char* OpenglFragmentShader = R"###(
#version 450 core
out vec4 fragColor;
in vec4 mColor;
in vec2 mTexCoord;
uniform sampler2D uTexture;

void main(void) {
    fragColor = texture(uTexture, mTexCoord) * mColor; 
})###";



// NOTE(Momo): Attributes
enum { 
    OpenglAtb_Model,    // 0 
    OpenglAtb_Colors,   // 1
    OpenglAtb_Texture1, // 2
    OpenglAtb_Texture2, // 3
    OpenglAtb_Texture3, // 4
    OpenglAtb_Texture4, // 5
    OpenglAtb_Transform1, // 6
    OpenglAtb_Transform2, // 7
    OpenglAtb_Transform3, // 8
    OpenglAtb_Transform4  // 9
};

// NOTE(Momo): VAO bindings
enum {
    OpenglVaoBind_Model,
    OpenglVaoBind_Colors,
    OpenglVaoBind_Texture,
    OpenglVaoBind_Transform
};

enum {
    OpenglPredefTexture_Dummy,
    OpenglPredefTexture_Blank,
    
    OpenglPredefTexture_Max
};

struct opengl {
    // Bindings that needs to be filled by platform
    OpenglFunctionPtr(glEnable);
    OpenglFunctionPtr(glDisable); 
    OpenglFunctionPtr(glViewport);
    OpenglFunctionPtr(glScissor);
    OpenglFunctionPtr(glCreateShader);
    OpenglFunctionPtr(glCompileShader);
    OpenglFunctionPtr(glShaderSource);
    OpenglFunctionPtr(glAttachShader);
    OpenglFunctionPtr(glDeleteShader);
    OpenglFunctionPtr(glClear);
    OpenglFunctionPtr(glClearColor);
    OpenglFunctionPtr(glCreateBuffers);
    OpenglFunctionPtr(glNamedBufferStorage);
    OpenglFunctionPtr(glCreateVertexArrays);
    OpenglFunctionPtr(glVertexArrayVertexBuffer);
    OpenglFunctionPtr(glEnableVertexArrayAttrib);
    OpenglFunctionPtr(glVertexArrayAttribFormat);
    OpenglFunctionPtr(glVertexArrayAttribBinding);
    OpenglFunctionPtr(glVertexArrayBindingDivisor);
    OpenglFunctionPtr(glBlendFunc);
    OpenglFunctionPtr(glCreateProgram);
    OpenglFunctionPtr(glLinkProgram);
    OpenglFunctionPtr(glGetProgramiv);
    OpenglFunctionPtr(glGetProgramInfoLog);
    OpenglFunctionPtr(glVertexArrayElementBuffer);
    OpenglFunctionPtr(glCreateTextures);
    OpenglFunctionPtr(glTextureStorage2D);
    OpenglFunctionPtr(glTextureSubImage2D);
    OpenglFunctionPtr(glBindTexture);
    OpenglFunctionPtr(glTexParameteri);
    OpenglFunctionPtr(glBindVertexArray);
    OpenglFunctionPtr(glDrawElementsInstancedBaseInstance);
    OpenglFunctionPtr(glGetUniformLocation);
    OpenglFunctionPtr(glProgramUniformMatrix4fv);
    OpenglFunctionPtr(glNamedBufferSubData);
    OpenglFunctionPtr(glUseProgram);
    OpenglFunctionPtr(glDeleteTextures);
    OpenglFunctionPtr(glDebugMessageCallbackARB);
    
    b8 IsInitialized;
    
    GLuint Buffers[OpenglVbo_Max]; 
    GLuint Shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint Model; 
    
    // NOTE(Momo): A table mapping  between
    // 'game texture handler' <-> 'opengl texture handler'  
    // Index 0 will always be an invalid 'dummy texture
    // Index 1 will always be a blank texture for items with no texture (but has colors)
    List<GLuint> Textures;
    
    v2u WindowDimensions;
    v2u DesignDimensions;
    aabb2u RenderRegion;
};

static inline void 
Opengl_AttachShader(opengl* Opengl, u32 Program, u32 Type, char* Code) {
    GLuint Shader = Opengl->glCreateShader(Type);
    Opengl->glShaderSource(Shader, 1, &Code, NULL);
    Opengl->glCompileShader(Shader);
    Opengl->glAttachShader(Program, Shader);
    Opengl->glDeleteShader(Shader);
}

// TODO: Change name to OpenglAliugnViewport
static inline void 
Opengl_AlignViewport(opengl* Opengl) 
{
    aabb2u Region = Renderer_CalcRenderRegion(Opengl->WindowDimensions.w, 
                                              Opengl->WindowDimensions.h, 
                                              Opengl->DesignDimensions.w, 
                                              Opengl->DesignDimensions.h);
    
    u32 x, y, w, h;
    x = Region.min.x;
    y = Region.min.y;
    w = width(Region);
    h = height(Region);
    
    Opengl->glViewport(x, y, w, h);
    Opengl->glScissor(x, y, w, h);
    
    // NOTE(Momo): Cache this to make calculations 
    // for window-space to render-space more CPU friendly.
    Opengl->RenderRegion = Region;
}




static inline void 
Opengl_Resize(opengl* Opengl,  
              u16 WindowWidth, 
              u16 WindowHeight) 
{
    Opengl->WindowDimensions.w = WindowWidth;
    Opengl->WindowDimensions.h = WindowHeight;
    Opengl_AlignViewport(Opengl);
}

static inline void 
Opengl_AddPredefTextures(opengl* Opengl) {
    struct pixel { u8 E[4]; };
    
    
    // NOTE(Momo): Dummy texture setup
    {
        pixel Pixels[4] = {
            { 125, 125, 125, 255 },
            { 255, 255, 255, 255 },
            { 255, 255, 255, 255 },
            { 125, 125, 125, 255 },
        };
        
        GLuint DummyTexture;
        Opengl->glCreateTextures(GL_TEXTURE_2D, 1, &DummyTexture);
        Opengl->glTextureStorage2D(DummyTexture, 1, GL_RGBA8, 2, 2);
        Opengl->glTextureSubImage2D(DummyTexture, 
                                    0, 0, 0, 
                                    2, 2, 
                                    GL_RGBA, 
                                    GL_UNSIGNED_BYTE, 
                                    &Pixels);
        Opengl->Textures.push_item(DummyTexture);
    }
    
    // NOTE(Momo): Blank texture setup
    {
        pixel Pixel = { 255, 255, 255, 255 };
        GLuint BlankTexture;
        Opengl->glCreateTextures(GL_TEXTURE_2D, 1, &BlankTexture);
        Opengl->glTextureStorage2D(BlankTexture, 1, GL_RGBA8, 1, 1);
        Opengl->glTextureSubImage2D(BlankTexture, 
                                    0, 0, 0, 
                                    1, 1, 
                                    GL_RGBA, GL_UNSIGNED_BYTE, 
                                    &Pixel);
        Opengl->Textures.push_item(BlankTexture);
    }
    
    
}

static inline b8
Opengl_Init(opengl* Opengl,
            Arena* arena,
            v2u WindowDimensions) 
{
    Opengl->Textures.alloc(arena, Opengl_Max_Textures);
    Opengl->DesignDimensions = WindowDimensions;
    Opengl->WindowDimensions = WindowDimensions;
    
    Opengl->glEnable(GL_DEPTH_TEST);
    Opengl->glEnable(GL_SCISSOR_TEST);
    
    Opengl->glClear(GL_C4f_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Opengl->glClearColor(0.f, 0.f, 0.f, 0.f);
    
    Opengl_AlignViewport(Opengl);
    
    
    // NOTE(Momo): Setup VBO
    Opengl->glCreateBuffers(OpenglVbo_Max, Opengl->Buffers);
    Opengl->glNamedBufferStorage(Opengl->Buffers[OpenglVbo_Model], 
                                 sizeof(QuadModel), 
                                 QuadModel, 
                                 0);
    
    Opengl->glNamedBufferStorage(Opengl->Buffers[OpenglVbo_Indices], 
                                 sizeof(QuadIndices), 
                                 QuadIndices, 
                                 0);
    
    Opengl->glNamedBufferStorage(Opengl->Buffers[OpenglVbo_Texture], 
                                 sizeof(v2f) * 4 * Opengl_Max_Entities, 
                                 nullptr, 
                                 GL_DYNAMIC_STORAGE_BIT);
    
    Opengl->glNamedBufferStorage(Opengl->Buffers[OpenglVbo_Colors], 
                                 sizeof(v4f) * Opengl_Max_Entities, 
                                 nullptr, 
                                 GL_DYNAMIC_STORAGE_BIT);
    
    Opengl->glNamedBufferStorage(Opengl->Buffers[OpenglVbo_Transform], 
                                 sizeof(m44f) * Opengl_Max_Entities, 
                                 nullptr, 
                                 GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    Opengl->glCreateVertexArrays(1, &Opengl->Model);
    Opengl->glVertexArrayVertexBuffer(Opengl->Model, 
                                      OpenglVaoBind_Model, 
                                      Opengl->Buffers[OpenglVbo_Model], 
                                      0, 
                                      sizeof(f32)*3);
    
    Opengl->glVertexArrayVertexBuffer(Opengl->Model, 
                                      OpenglVaoBind_Texture, 
                                      Opengl->Buffers[OpenglVbo_Texture], 
                                      0, 
                                      sizeof(f32) * 8);
    
    Opengl->glVertexArrayVertexBuffer(Opengl->Model, 
                                      OpenglVaoBind_Colors, 
                                      Opengl->Buffers[OpenglVbo_Colors],  
                                      0, 
                                      sizeof(v4f));
    
    Opengl->glVertexArrayVertexBuffer(Opengl->Model, 
                                      OpenglVaoBind_Transform, 
                                      Opengl->Buffers[OpenglVbo_Transform], 
                                      0, 
                                      sizeof(m44f));
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Model); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Model, 
                                      3, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      0);
    
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Model, 
                                       OpenglVaoBind_Model);
    
    // aColor
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Colors); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Colors, 
                                      4, 
                                      GL_FLOAT, GL_FALSE, 0);
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Colors, 
                                       OpenglVaoBind_Colors);
    
    Opengl->glVertexArrayBindingDivisor(Opengl->Model, OpenglVaoBind_Colors, 1); 
    
    // aTexCoord
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Texture1); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Texture1, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 0);
    
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Texture2); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Texture2, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 1);
    
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Texture3); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Texture3, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 2);
    
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Texture4); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Texture4, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 3);
    
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Texture1, 
                                       OpenglVaoBind_Texture);
    
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Texture2, 
                                       OpenglVaoBind_Texture);
    
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Texture3, 
                                       OpenglVaoBind_Texture);
    
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Texture4, 
                                       OpenglVaoBind_Texture);
    
    Opengl->glVertexArrayBindingDivisor(Opengl->Model, 
                                        OpenglVaoBind_Texture, 
                                        1); 
    
    
    // aTransform
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Transform1); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Transform1, 
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 0 * 4);
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Transform2);
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Transform2, 
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 1 * 4);
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Transform3); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Transform3, 
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 2 * 4);
    Opengl->glEnableVertexArrayAttrib(Opengl->Model, OpenglAtb_Transform4); 
    Opengl->glVertexArrayAttribFormat(Opengl->Model, 
                                      OpenglAtb_Transform4,
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 3 * 4);
    
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Transform1, 
                                       OpenglVaoBind_Transform);
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Transform2, 
                                       OpenglVaoBind_Transform);
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Transform3, 
                                       OpenglVaoBind_Transform);
    Opengl->glVertexArrayAttribBinding(Opengl->Model, 
                                       OpenglAtb_Transform4, 
                                       OpenglVaoBind_Transform);
    
    Opengl->glVertexArrayBindingDivisor(Opengl->Model, 
                                        OpenglVaoBind_Transform, 
                                        1); 
    
    // NOTE(Momo): Alpha blend
    Opengl->glEnable(GL_BLEND);
    Opengl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // NOTE(Momo): Setup indices
    Opengl->glVertexArrayElementBuffer(Opengl->Model, 
                                       Opengl->Buffers[OpenglVbo_Indices]);
    
    
    
    // NOTE(Momo): Setup Shader Program
    Opengl->Shader = Opengl->glCreateProgram();
    Opengl_AttachShader(Opengl, 
                        Opengl->Shader, 
                        GL_VERTEX_SHADER, 
                        (char*)OpenglVertexShader);
    Opengl_AttachShader(Opengl, 
                        Opengl->Shader, 
                        GL_FRAGMENT_SHADER, 
                        (char*)OpenglFragmentShader);
    
    Opengl->glLinkProgram(Opengl->Shader);
    
    GLint Result;
    Opengl->glGetProgramiv(Opengl->Shader, GL_LINK_STATUS, &Result);
    if (Result != GL_TRUE) {
        char msg[KIBIBYTE];
        Opengl->glGetProgramInfoLog(Opengl->Shader, KIBIBYTE, nullptr, msg);
        // TODO(Momo): Log?
        return false;
    }
    Opengl_AddPredefTextures(Opengl);
    Opengl->IsInitialized = true;
    return true;
}


static inline void 
Opengl_DrawInstances(opengl* Opengl, 
                     GLuint Texture, 
                     u32 InstancesToDraw, 
                     u32 IndexToDrawFrom) 
{
    ASSERT(InstancesToDraw + IndexToDrawFrom < Opengl_Max_Entities);
    if (InstancesToDraw > 0) {
        Opengl->glBindTexture(GL_TEXTURE_2D, Texture);
        Opengl->glTexParameteri(GL_TEXTURE_2D, 
                                GL_TEXTURE_MIN_FILTER, 
                                GL_NEAREST);
        Opengl->glTexParameteri(GL_TEXTURE_2D, 
                                GL_TEXTURE_MAG_FILTER, 
                                GL_NEAREST);
        Opengl->glEnable(GL_BLEND);
        Opengl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Opengl->glBindVertexArray(Opengl->Model);
        Opengl->glUseProgram(Opengl->Shader);
        
        Opengl->glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                                    6, 
                                                    GL_UNSIGNED_BYTE, 
                                                    nullptr, 
                                                    InstancesToDraw,
                                                    IndexToDrawFrom);
    }
}


static inline renderer_texture_handle
Opengl_AddTexture(opengl* Opengl,
                  u32 Width,
                  u32 Height,
                  void* Pixels) 
{
    renderer_texture_handle Ret = {};
    
    if (Opengl->Textures.remaining() == 0) {
        Ret.Success = false;
        Ret.Id = 0;
        return Ret;
    }
    
    
    GLuint Entry;
    
    Opengl->glCreateTextures(GL_TEXTURE_2D, 
                             1, 
                             &Entry);
    
    Opengl->glTextureStorage2D(Entry, 
                               1, 
                               GL_RGBA8, 
                               Width, 
                               Height);
    
    Opengl->glTextureSubImage2D(Entry, 
                                0, 
                                0, 
                                0, 
                                Width, 
                                Height, 
                                GL_RGBA, 
                                GL_UNSIGNED_BYTE, 
                                Pixels);
    
    Ret.Id = Opengl->Textures.count;
    Ret.Success = true;
    Opengl->Textures.push_item(Entry);
    return Ret;
}

static inline void
Opengl_ClearTextures(opengl* Opengl) {
    Opengl->glDeleteTextures(Opengl->Textures.count, 
                             Opengl->Textures.data);
    Opengl->Textures.clear();
    Opengl_AddPredefTextures(Opengl);
}


static inline void
Opengl_Render(opengl* Opengl, Mailbox* Commands) 
{
    // TODO(Momo): Better way to do this without binding texture first?
    GLuint CurrentTexture = 0;
    u32 InstancesToDrawCount = 0;
    u32 LastDrawnInstanceIndex = 0;
    u32 CurrentInstanceIndex = 0;
    
    Opengl->glClear(GL_C4f_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (u32 i = 0; i < Commands->entry_count; ++i) {
        Mailbox_Entry_Header* Entry = Commands->get_entry(i);
        
        switch(Entry->type) {
            case RendererCommandType_SetDesignResolution: {
                auto* Data = (renderer_command_set_design_resolution*)
                    Commands->get_entry_data(Entry);
                
                Opengl->DesignDimensions.w = Data->Width;
                Opengl->DesignDimensions.h = Data->Height;
                Opengl_AlignViewport(Opengl);
            } break;
            case RendererCommandType_SetBasis: {
                auto* Data = (renderer_command_set_basis*)
                    Commands->get_entry_data(Entry);
                
                Opengl_DrawInstances(Opengl, 
                                     CurrentTexture, 
                                     InstancesToDrawCount, 
                                     LastDrawnInstanceIndex);
                LastDrawnInstanceIndex += InstancesToDrawCount;
                InstancesToDrawCount = 0;
                
                auto Result = transpose(Data->Basis);
                GLint uProjectionLoc = Opengl->glGetUniformLocation(Opengl->Shader,
                                                                    "uProjection");
                
                Opengl->glProgramUniformMatrix4fv(Opengl->Shader, 
                                                  uProjectionLoc, 
                                                  1, 
                                                  GL_FALSE, 
                                                  (const GLfloat*)&Result);
            } break;
            case RendererCommandType_ClearColor: {
                auto* Data = (renderer_command_clear_color*)
                    Commands->get_entry_data(Entry);
                Opengl->glClearColor(Data->Colors.r, 
                                     Data->Colors.g, 
                                     Data->Colors.b, 
                                     Data->Colors.a);
            } break;
            case RendererCommandType_DrawQuad: {
                auto* Data = (renderer_command_draw_quad*)
                    Commands->get_entry_data(Entry);
                
                GLuint OpenglTextureHandle = *(Opengl->Textures + OpenglPredefTexture_Blank);
                
                // NOTE(Momo): If the currently set texture is not same as the 
                // currently processed texture, batch draw all instances before 
                // the current instance.
                if (CurrentTexture != OpenglTextureHandle) {
                    Opengl_DrawInstances(Opengl, 
                                         CurrentTexture, 
                                         InstancesToDrawCount, 
                                         LastDrawnInstanceIndex);
                    LastDrawnInstanceIndex += InstancesToDrawCount;
                    InstancesToDrawCount = 0;
                    CurrentTexture = OpenglTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                Opengl->glNamedBufferSubData(Opengl->Buffers[OpenglVbo_Colors], 
                                             CurrentInstanceIndex * sizeof(v4f),
                                             sizeof(v4f), 
                                             &Data->Colors);
                
                Opengl->glNamedBufferSubData(Opengl->Buffers[OpenglVbo_Texture],
                                             CurrentInstanceIndex * sizeof(quad2f),
                                             sizeof(QuadUV),
                                             &QuadUV);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f Transform = transpose(Data->Transform);
                Opengl->glNamedBufferSubData(Opengl->Buffers[OpenglVbo_Transform], 
                                             CurrentInstanceIndex* sizeof(m44f), 
                                             sizeof(m44f), 
                                             &Transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++InstancesToDrawCount;
                ++CurrentInstanceIndex;
            } break;
            case RendererCommandType_DrawTexturedQuad: {
                auto* Data = (renderer_command_draw_textured_quad*)
                    Commands->get_entry_data(Entry);
                
                GLuint OpenglTextureHandle = *(Opengl->Textures + Data->TextureHandle.Id); 
                
                // NOTE(Momo): If the currently set texture is not same as the currently
                // processed texture, batch draw all instances before the current instance.
                if (CurrentTexture != OpenglTextureHandle) {
                    Opengl_DrawInstances(Opengl, 
                                         CurrentTexture, 
                                         InstancesToDrawCount, 
                                         LastDrawnInstanceIndex);
                    LastDrawnInstanceIndex += InstancesToDrawCount;
                    InstancesToDrawCount = 0;
                    CurrentTexture = OpenglTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                Opengl->glNamedBufferSubData(Opengl->Buffers[OpenglVbo_Colors], 
                                             CurrentInstanceIndex * sizeof(v4f),
                                             sizeof(v4f), 
                                             &Data->Colors);
                
                Opengl->glNamedBufferSubData(Opengl->Buffers[OpenglVbo_Texture],
                                             CurrentInstanceIndex * sizeof(quad2f),
                                             sizeof(quad2f),
                                             &Data->TextureCoords);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f Transform = transpose(Data->Transform);
                Opengl->glNamedBufferSubData(Opengl->Buffers[OpenglVbo_Transform], 
                                             CurrentInstanceIndex* sizeof(m44f), 
                                             sizeof(m44f), 
                                             &Transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++InstancesToDrawCount;
                ++CurrentInstanceIndex;
                
            } break;
        }
    }
    
    Opengl_DrawInstances(Opengl, CurrentTexture, InstancesToDrawCount, LastDrawnInstanceIndex);
    
}



#endif
