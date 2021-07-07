#ifndef __RENDERER_OPENGL__
#define __RENDERER_OPENGL__

#include "game_renderer.h"

// Configuration
#define OPENGL_MAX_TEXTURES 8
#define OPENGL_MAX_ENTITIES 4096

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

#define OPENGL_FUNCTION_DECL(Name) opengl_func_##Name
#define OPENGL_FUNCTION_PTR(Name) OPENGL_FUNCTION_DECL(Name)* Name
#define OPENGL_DEBUG_CALLBACK_FUNC(Name) void Name(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *msg,const void *userParam)
typedef OPENGL_DEBUG_CALLBACK_FUNC(GLDEBUGPROC);

typedef void    OPENGL_FUNCTION_DECL(glEnable)(GLenum cap);
typedef void    OPENGL_FUNCTION_DECL(glDisable)(GLenum cap);
typedef void    OPENGL_FUNCTION_DECL(glViewport)(GLint x, 
                                                 GLint y, 
                                                 GLsizei width, 
                                                 GLsizei height);
typedef void    OPENGL_FUNCTION_DECL(glScissor)(GLint x, 
                                                GLint y, 
                                                GLsizei width, 
                                                GLsizei height); 
typedef GLuint  OPENGL_FUNCTION_DECL(glCreateShader)(GLenum type);
typedef void    OPENGL_FUNCTION_DECL(glCompileShader)(GLuint program);
typedef void    OPENGL_FUNCTION_DECL(glShaderSource)(GLuint shader, 
                                                     GLsizei count, 
                                                     GLchar** string, 
                                                     GLint* length);
typedef void    OPENGL_FUNCTION_DECL(glAttachShader)(GLuint program, GLuint shader);
typedef void    OPENGL_FUNCTION_DECL(glDeleteShader)(GLuint program);
typedef void    OPENGL_FUNCTION_DECL(glClear)(GLbitfield mask);
typedef void    OPENGL_FUNCTION_DECL(glClearColor)(GLclampf r, 
                                                   GLclampf g, 
                                                   GLclampf b, 
                                                   GLclampf a);
typedef void    OPENGL_FUNCTION_DECL(glCreateBuffers)(GLsizei n, GLuint* buffers);
typedef void    OPENGL_FUNCTION_DECL(glNamedBufferStorage)(GLuint buffer, 
                                                           GLsizeiptr size, 
                                                           const void* data, 
                                                           GLbitfield flags);
typedef void    OPENGL_FUNCTION_DECL(glCreateVertexArrays)(GLsizei n, GLuint* arrays);
typedef void    OPENGL_FUNCTION_DECL(glVertexArrayVertexBuffer)(GLuint vaobj, 
                                                                GLuint bindingindex, 
                                                                GLuint buffer, 
                                                                GLintptr offset, 
                                                                GLsizei stride);
typedef void    OPENGL_FUNCTION_DECL(glEnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
typedef void    OPENGL_FUNCTION_DECL(glVertexArrayAttribFormat)(GLuint vaobj,
                                                                GLuint attribindex,
                                                                GLint size,
                                                                GLenum type,
                                                                GLboolean normalized,
                                                                GLuint relativeoffset);
typedef void    OPENGL_FUNCTION_DECL(glVertexArrayAttribBinding)(GLuint vaobj,
                                                                 GLuint attribindex,
                                                                 GLuint bindingindex);
typedef void    OPENGL_FUNCTION_DECL(glVertexArrayBindingDivisor)(GLuint vaobj,
                                                                  GLuint bindingindex,
                                                                  GLuint divisor);
typedef void    OPENGL_FUNCTION_DECL(glBlendFunc)(GLenum sfactor, GLenum dfactor);
typedef void    OPENGL_FUNCTION_DECL(glVertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
typedef GLuint  OPENGL_FUNCTION_DECL(glCreateProgram)();
typedef void    OPENGL_FUNCTION_DECL(glLinkProgram)(GLuint program);
typedef void    OPENGL_FUNCTION_DECL(glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
typedef void    OPENGL_FUNCTION_DECL(glGetProgramInfoLog)(GLuint program, 
                                                          GLsizei maxLength,
                                                          GLsizei* length,
                                                          GLchar* infoLog);
typedef void    OPENGL_FUNCTION_DECL(glCreateTextures)(GLenum target, 
                                                       GLsizei n, 
                                                       GLuint* textures);
typedef void    OPENGL_FUNCTION_DECL(glTextureStorage2D)(GLuint texture,
                                                         GLsizei levels,
                                                         GLenum internalformat,
                                                         GLsizei width,
                                                         GLsizei height);
typedef void    OPENGL_FUNCTION_DECL(glTextureSubImage2D)(GLuint texture,
                                                          GLint level,
                                                          GLint xoffset,
                                                          GLint yoffset,
                                                          GLsizei width,
                                                          GLsizei height,
                                                          GLenum format,
                                                          GLenum type,
                                                          const void* pixels);
typedef void    OPENGL_FUNCTION_DECL(glBindTexture)(GLenum target, GLuint texture);
typedef void    OPENGL_FUNCTION_DECL(glTexParameteri)(GLenum target ,GLenum pname, GLint param);
typedef void    OPENGL_FUNCTION_DECL(glBindVertexArray)(GLuint array);
typedef void    OPENGL_FUNCTION_DECL(glDrawElementsInstancedBaseInstance)(GLenum mode,
                                                                          GLsizei count,
                                                                          GLenum type,
                                                                          const void* indices,
                                                                          GLsizei instancecount,
                                                                          GLuint baseinstance);
typedef void    OPENGL_FUNCTION_DECL(glUseProgram)(GLuint program);
typedef void    OPENGL_FUNCTION_DECL(glNamedBufferSubData)(GLuint buffer,
                                                           GLintptr offset,
                                                           GLsizeiptr size,
                                                           const void* data);
typedef GLint   OPENGL_FUNCTION_DECL(glGetUniformLocation)(GLuint program,
                                                           const GLchar* name);
typedef void    OPENGL_FUNCTION_DECL(glProgramUniformMatrix4fv)(GLuint program,
                                                                GLint location,
                                                                GLsizei count,
                                                                GLboolean transpose,
                                                                const GLfloat* value);
typedef void    OPENGL_FUNCTION_DECL(glUseProgram)(GLuint program);
typedef void    OPENGL_FUNCTION_DECL(glDeleteTextures)(GLsizei n, 
                                                       const GLuint* textures);
typedef void    OPENGL_FUNCTION_DECL(glDebugMessageCallbackARB)(GLDEBUGPROC *callback, 
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

// NOTE(Momo): buffers
enum {
    OPENGL_VBO_MODEL,
    OPENGL_VBO_INDICES,
    OPENGL_VBO_COLORS,
    OPENGL_VBO_TEXTURE,
    OPENGL_VBO_TRANSFORM,
    OPENGL_VBO_MAX
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
    OPENGL_ATB_MODEL,    // 0 
    OPENGL_ATB_COLORS,   // 1
    OPENGL_ATB_TEXTURE_1, // 2
    OPENGL_ATB_TEXTURE_2, // 3
    OPENGL_ATB_TEXTURE_3, // 4
    OPENGL_ATB_TEXTURE_4, // 5
    OPENGL_ATB_TRANSFORM_1, // 6
    OPENGL_ATB_TRANSFORM_2, // 7
    OPENGL_ATB_TRANSFORM_3, // 8
    OPENGL_ATB_TRANSFORM_4  // 9
};

// NOTE(Momo): VAO bindings
enum {
    OPENGL_VAO_BIND_MODEL,
    OPENGL_VAO_BIND_COLORS,
    OPENGL_VAO_BIND_TEXTURE,
    OPENGL_VAO_BIND_TRANSFORM
};

enum {
    OPENGL_PREDEF_TEXTURE_DUMMY,
    OPENGL_PREDEF_TEXTURE_BLANK,
    
    OPENGL_PREDEF_TEXTURE_MAX
};

struct Opengl {
    // Bindings that needs to be filled by platform
    OPENGL_FUNCTION_PTR(glEnable);
    OPENGL_FUNCTION_PTR(glDisable); 
    OPENGL_FUNCTION_PTR(glViewport);
    OPENGL_FUNCTION_PTR(glScissor);
    OPENGL_FUNCTION_PTR(glCreateShader);
    OPENGL_FUNCTION_PTR(glCompileShader);
    OPENGL_FUNCTION_PTR(glShaderSource);
    OPENGL_FUNCTION_PTR(glAttachShader);
    OPENGL_FUNCTION_PTR(glDeleteShader);
    OPENGL_FUNCTION_PTR(glClear);
    OPENGL_FUNCTION_PTR(glClearColor);
    OPENGL_FUNCTION_PTR(glCreateBuffers);
    OPENGL_FUNCTION_PTR(glNamedBufferStorage);
    OPENGL_FUNCTION_PTR(glCreateVertexArrays);
    OPENGL_FUNCTION_PTR(glVertexArrayVertexBuffer);
    OPENGL_FUNCTION_PTR(glEnableVertexArrayAttrib);
    OPENGL_FUNCTION_PTR(glVertexArrayAttribFormat);
    OPENGL_FUNCTION_PTR(glVertexArrayAttribBinding);
    OPENGL_FUNCTION_PTR(glVertexArrayBindingDivisor);
    OPENGL_FUNCTION_PTR(glBlendFunc);
    OPENGL_FUNCTION_PTR(glCreateProgram);
    OPENGL_FUNCTION_PTR(glLinkProgram);
    OPENGL_FUNCTION_PTR(glGetProgramiv);
    OPENGL_FUNCTION_PTR(glGetProgramInfoLog);
    OPENGL_FUNCTION_PTR(glVertexArrayElementBuffer);
    OPENGL_FUNCTION_PTR(glCreateTextures);
    OPENGL_FUNCTION_PTR(glTextureStorage2D);
    OPENGL_FUNCTION_PTR(glTextureSubImage2D);
    OPENGL_FUNCTION_PTR(glBindTexture);
    OPENGL_FUNCTION_PTR(glTexParameteri);
    OPENGL_FUNCTION_PTR(glBindVertexArray);
    OPENGL_FUNCTION_PTR(glDrawElementsInstancedBaseInstance);
    OPENGL_FUNCTION_PTR(glGetUniformLocation);
    OPENGL_FUNCTION_PTR(glProgramUniformMatrix4fv);
    OPENGL_FUNCTION_PTR(glNamedBufferSubData);
    OPENGL_FUNCTION_PTR(glUseProgram);
    OPENGL_FUNCTION_PTR(glDeleteTextures);
    OPENGL_FUNCTION_PTR(glDebugMessageCallbackARB);
    
    b8 is_initialized;
    
    GLuint buffers[OPENGL_VBO_MAX]; 
    GLuint shader;
    
    // NOTE(Momo): We only need one blueprint which is a 1x1 square.
    GLuint model; 
    
    // NOTE(Momo): A table mapping  between
    // 'game texture handler' <-> 'opengl texture handler'  
    // Index 0 will always be an invalid 'dummy texture
    // Index 1 will always be a blank texture for items with no texture (but has colors)
    List<GLuint> textures;
    
    v2u window_dimensions;
    v2u design_dimensions;
    aabb2u render_region;
};

static inline void 
Opengl_AttachShader(Opengl* opengl, u32 Program, u32 Type, char* Code) {
    GLuint shader = opengl->glCreateShader(Type);
    opengl->glShaderSource(shader, 1, &Code, NULL);
    opengl->glCompileShader(shader);
    opengl->glAttachShader(Program, shader);
    opengl->glDeleteShader(shader);
}

// TODO: Change name to OpenglAliugnViewport
static inline void 
Opengl_AlignViewport(Opengl* opengl) 
{
    aabb2u Region = Renderer_CalcRenderRegion(opengl->window_dimensions.w, 
                                              opengl->window_dimensions.h, 
                                              opengl->design_dimensions.w, 
                                              opengl->design_dimensions.h);
    
    u32 x, y, w, h;
    x = Region.min.x;
    y = Region.min.y;
    w = width(Region);
    h = height(Region);
    
    opengl->glViewport(x, y, w, h);
    opengl->glScissor(x, y, w, h);
    
    // NOTE(Momo): Cache this to make calculations 
    // for window-space to render-space more CPU friendly.
    opengl->render_region = Region;
}




static inline void 
Opengl_Resize(Opengl* opengl,  
              u16 WindowWidth, 
              u16 WindowHeight) 
{
    opengl->window_dimensions.w = WindowWidth;
    opengl->window_dimensions.h = WindowHeight;
    Opengl_AlignViewport(opengl);
}

static inline void 
Opengl_AddPredefTextures(Opengl* opengl) {
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
        opengl->glCreateTextures(GL_TEXTURE_2D, 1, &DummyTexture);
        opengl->glTextureStorage2D(DummyTexture, 1, GL_RGBA8, 2, 2);
        opengl->glTextureSubImage2D(DummyTexture, 
                                    0, 0, 0, 
                                    2, 2, 
                                    GL_RGBA, 
                                    GL_UNSIGNED_BYTE, 
                                    &Pixels);
        opengl->textures.push_item(DummyTexture);
    }
    
    // NOTE(Momo): Blank texture setup
    {
        pixel Pixel = { 255, 255, 255, 255 };
        GLuint BlankTexture;
        opengl->glCreateTextures(GL_TEXTURE_2D, 1, &BlankTexture);
        opengl->glTextureStorage2D(BlankTexture, 1, GL_RGBA8, 1, 1);
        opengl->glTextureSubImage2D(BlankTexture, 
                                    0, 0, 0, 
                                    1, 1, 
                                    GL_RGBA, GL_UNSIGNED_BYTE, 
                                    &Pixel);
        opengl->textures.push_item(BlankTexture);
    }
    
    
}

static inline b8
Opengl_Init(Opengl* opengl,
            Arena* arena,
            v2u window_dimensions) 
{
    opengl->textures.alloc(arena, OPENGL_MAX_TEXTURES);
    opengl->design_dimensions = window_dimensions;
    opengl->window_dimensions = window_dimensions;
    
    opengl->glEnable(GL_DEPTH_TEST);
    opengl->glEnable(GL_SCISSOR_TEST);
    
    opengl->glClear(GL_C4f_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    opengl->glClearColor(0.f, 0.f, 0.f, 0.f);
    
    Opengl_AlignViewport(opengl);
    
    
    // NOTE(Momo): Setup VBO
    opengl->glCreateBuffers(OPENGL_VBO_MAX, opengl->buffers);
    opengl->glNamedBufferStorage(opengl->buffers[OPENGL_VBO_MODEL], 
                                 sizeof(QuadModel), 
                                 QuadModel, 
                                 0);
    
    opengl->glNamedBufferStorage(opengl->buffers[OPENGL_VBO_INDICES], 
                                 sizeof(QuadIndices), 
                                 QuadIndices, 
                                 0);
    
    opengl->glNamedBufferStorage(opengl->buffers[OPENGL_VBO_TEXTURE], 
                                 sizeof(v2f) * 4 * OPENGL_MAX_ENTITIES, 
                                 nullptr, 
                                 GL_DYNAMIC_STORAGE_BIT);
    
    opengl->glNamedBufferStorage(opengl->buffers[OPENGL_VBO_COLORS], 
                                 sizeof(v4f) * OPENGL_MAX_ENTITIES, 
                                 nullptr, 
                                 GL_DYNAMIC_STORAGE_BIT);
    
    opengl->glNamedBufferStorage(opengl->buffers[OPENGL_VBO_TRANSFORM], 
                                 sizeof(m44f) * OPENGL_MAX_ENTITIES, 
                                 nullptr, 
                                 GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    opengl->glCreateVertexArrays(1, &opengl->model);
    opengl->glVertexArrayVertexBuffer(opengl->model, 
                                      OPENGL_VAO_BIND_MODEL, 
                                      opengl->buffers[OPENGL_VBO_MODEL], 
                                      0, 
                                      sizeof(f32)*3);
    
    opengl->glVertexArrayVertexBuffer(opengl->model, 
                                      OPENGL_VAO_BIND_TEXTURE, 
                                      opengl->buffers[OPENGL_VBO_TEXTURE], 
                                      0, 
                                      sizeof(f32) * 8);
    
    opengl->glVertexArrayVertexBuffer(opengl->model, 
                                      OPENGL_VAO_BIND_COLORS, 
                                      opengl->buffers[OPENGL_VBO_COLORS],  
                                      0, 
                                      sizeof(v4f));
    
    opengl->glVertexArrayVertexBuffer(opengl->model, 
                                      OPENGL_VAO_BIND_TRANSFORM, 
                                      opengl->buffers[OPENGL_VBO_TRANSFORM], 
                                      0, 
                                      sizeof(m44f));
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_MODEL); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_MODEL, 
                                      3, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      0);
    
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_MODEL, 
                                       OPENGL_VAO_BIND_MODEL);
    
    // aColor
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_COLORS); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_COLORS, 
                                      4, 
                                      GL_FLOAT, GL_FALSE, 0);
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_COLORS, 
                                       OPENGL_VAO_BIND_COLORS);
    
    opengl->glVertexArrayBindingDivisor(opengl->model, OPENGL_VAO_BIND_COLORS, 1); 
    
    // aTexCoord
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TEXTURE_1); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TEXTURE_1, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE,
                                      sizeof(v2f) * 0);
    
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TEXTURE_2); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TEXTURE_2, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 1);
    
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TEXTURE_3); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TEXTURE_3, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 2);
    
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TEXTURE_4); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TEXTURE_4, 
                                      2, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(v2f) * 3);
    
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TEXTURE_1, 
                                       OPENGL_VAO_BIND_TEXTURE);
    
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TEXTURE_2, 
                                       OPENGL_VAO_BIND_TEXTURE);
    
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TEXTURE_3, 
                                       OPENGL_VAO_BIND_TEXTURE);
    
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TEXTURE_4, 
                                       OPENGL_VAO_BIND_TEXTURE);
    
    opengl->glVertexArrayBindingDivisor(opengl->model, 
                                        OPENGL_VAO_BIND_TEXTURE, 
                                        1); 
    
    
    // aTransform
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TRANSFORM_1); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TRANSFORM_1, 
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 0 * 4);
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TRANSFORM_2);
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TRANSFORM_2, 
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 1 * 4);
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TRANSFORM_3); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TRANSFORM_3, 
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 2 * 4);
    opengl->glEnableVertexArrayAttrib(opengl->model, OPENGL_ATB_TRANSFORM_4); 
    opengl->glVertexArrayAttribFormat(opengl->model, 
                                      OPENGL_ATB_TRANSFORM_4,
                                      4, 
                                      GL_FLOAT, 
                                      GL_FALSE, 
                                      sizeof(f32) * 3 * 4);
    
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TRANSFORM_1, 
                                       OPENGL_VAO_BIND_TRANSFORM);
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TRANSFORM_2, 
                                       OPENGL_VAO_BIND_TRANSFORM);
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TRANSFORM_3, 
                                       OPENGL_VAO_BIND_TRANSFORM);
    opengl->glVertexArrayAttribBinding(opengl->model, 
                                       OPENGL_ATB_TRANSFORM_4, 
                                       OPENGL_VAO_BIND_TRANSFORM);
    
    opengl->glVertexArrayBindingDivisor(opengl->model, 
                                        OPENGL_VAO_BIND_TRANSFORM, 
                                        1); 
    
    // NOTE(Momo): alpha blend
    opengl->glEnable(GL_BLEND);
    opengl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // NOTE(Momo): Setup indices
    opengl->glVertexArrayElementBuffer(opengl->model, 
                                       opengl->buffers[OPENGL_VBO_INDICES]);
    
    
    
    // NOTE(Momo): Setup shader Program
    opengl->shader = opengl->glCreateProgram();
    Opengl_AttachShader(opengl, 
                        opengl->shader, 
                        GL_VERTEX_SHADER, 
                        (char*)OpenglVertexShader);
    Opengl_AttachShader(opengl, 
                        opengl->shader, 
                        GL_FRAGMENT_SHADER, 
                        (char*)OpenglFragmentShader);
    
    opengl->glLinkProgram(opengl->shader);
    
    GLint Result;
    opengl->glGetProgramiv(opengl->shader, GL_LINK_STATUS, &Result);
    if (Result != GL_TRUE) {
        char msg[KIBIBYTE];
        opengl->glGetProgramInfoLog(opengl->shader, KIBIBYTE, nullptr, msg);
        // TODO(Momo): Log?
        return false;
    }
    Opengl_AddPredefTextures(opengl);
    opengl->is_initialized = true;
    return true;
}


static inline void 
Opengl_DrawInstances(Opengl* opengl, 
                     GLuint texture, 
                     u32 InstancesToDraw, 
                     u32 IndexToDrawFrom) 
{
    ASSERT(InstancesToDraw + IndexToDrawFrom < OPENGL_MAX_ENTITIES);
    if (InstancesToDraw > 0) {
        opengl->glBindTexture(GL_TEXTURE_2D, texture);
        opengl->glTexParameteri(GL_TEXTURE_2D, 
                                GL_TEXTURE_MIN_FILTER, 
                                GL_NEAREST);
        opengl->glTexParameteri(GL_TEXTURE_2D, 
                                GL_TEXTURE_MAG_FILTER, 
                                GL_NEAREST);
        opengl->glEnable(GL_BLEND);
        opengl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        opengl->glBindVertexArray(opengl->model);
        opengl->glUseProgram(opengl->shader);
        
        opengl->glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                                    6, 
                                                    GL_UNSIGNED_BYTE, 
                                                    nullptr, 
                                                    InstancesToDraw,
                                                    IndexToDrawFrom);
    }
}


static inline renderer_texture_handle
Opengl_AddTexture(Opengl* opengl,
                  u32 Width,
                  u32 Height,
                  void* Pixels) 
{
    renderer_texture_handle Ret = {};
    
    if (opengl->textures.remaining() == 0) {
        Ret.Success = false;
        Ret.Id = 0;
        return Ret;
    }
    
    
    GLuint Entry;
    
    opengl->glCreateTextures(GL_TEXTURE_2D, 
                             1, 
                             &Entry);
    
    opengl->glTextureStorage2D(Entry, 
                               1, 
                               GL_RGBA8, 
                               Width, 
                               Height);
    
    opengl->glTextureSubImage2D(Entry, 
                                0, 
                                0, 
                                0, 
                                Width, 
                                Height, 
                                GL_RGBA, 
                                GL_UNSIGNED_BYTE, 
                                Pixels);
    
    Ret.Id = opengl->textures.count;
    Ret.Success = true;
    opengl->textures.push_item(Entry);
    return Ret;
}

static inline void
Opengl_ClearTextures(Opengl* opengl) {
    opengl->glDeleteTextures(opengl->textures.count, 
                             opengl->textures.data);
    opengl->textures.clear();
    Opengl_AddPredefTextures(opengl);
}


static inline void
Opengl_Render(Opengl* opengl, Mailbox* Commands) 
{
    // TODO(Momo): Better way to do this without binding texture first?
    GLuint CurrentTexture = 0;
    u32 InstancesToDrawCount = 0;
    u32 LastDrawnInstanceIndex = 0;
    u32 CurrentInstanceIndex = 0;
    
    opengl->glClear(GL_C4f_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (u32 i = 0; i < Commands->entry_count; ++i) {
        Mailbox_Entry_Header* Entry = Commands->get_entry(i);
        
        switch(Entry->type) {
            case RendererCommandType_SetDesignResolution: {
                auto* Data = (renderer_command_set_design_resolution*)
                    Commands->get_entry_data(Entry);
                
                opengl->design_dimensions.w = Data->Width;
                opengl->design_dimensions.h = Data->Height;
                Opengl_AlignViewport(opengl);
            } break;
            case RendererCommandType_SetBasis: {
                auto* Data = (renderer_command_set_basis*)
                    Commands->get_entry_data(Entry);
                
                Opengl_DrawInstances(opengl, 
                                     CurrentTexture, 
                                     InstancesToDrawCount, 
                                     LastDrawnInstanceIndex);
                LastDrawnInstanceIndex += InstancesToDrawCount;
                InstancesToDrawCount = 0;
                
                auto Result = transpose(Data->Basis);
                GLint uProjectionLoc = opengl->glGetUniformLocation(opengl->shader,
                                                                    "uProjection");
                
                opengl->glProgramUniformMatrix4fv(opengl->shader, 
                                                  uProjectionLoc, 
                                                  1, 
                                                  GL_FALSE, 
                                                  (const GLfloat*)&Result);
            } break;
            case RendererCommandType_ClearColor: {
                auto* Data = (renderer_command_clear_color*)
                    Commands->get_entry_data(Entry);
                opengl->glClearColor(Data->Colors.r, 
                                     Data->Colors.g, 
                                     Data->Colors.b, 
                                     Data->Colors.a);
            } break;
            case RendererCommandType_DrawQuad: {
                auto* Data = (renderer_command_draw_quad*)
                    Commands->get_entry_data(Entry);
                
                GLuint OpenglTextureHandle = *(opengl->textures + OPENGL_PREDEF_TEXTURE_BLANK);
                
                // NOTE(Momo): If the currently set texture is not same as the 
                // currently processed texture, batch draw all instances before 
                // the current instance.
                if (CurrentTexture != OpenglTextureHandle) {
                    Opengl_DrawInstances(opengl, 
                                         CurrentTexture, 
                                         InstancesToDrawCount, 
                                         LastDrawnInstanceIndex);
                    LastDrawnInstanceIndex += InstancesToDrawCount;
                    InstancesToDrawCount = 0;
                    CurrentTexture = OpenglTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                opengl->glNamedBufferSubData(opengl->buffers[OPENGL_VBO_COLORS], 
                                             CurrentInstanceIndex * sizeof(v4f),
                                             sizeof(v4f), 
                                             &Data->Colors);
                
                opengl->glNamedBufferSubData(opengl->buffers[OPENGL_VBO_TEXTURE],
                                             CurrentInstanceIndex * sizeof(quad2f),
                                             sizeof(QuadUV),
                                             &QuadUV);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f Transform = transpose(Data->Transform);
                opengl->glNamedBufferSubData(opengl->buffers[OPENGL_VBO_TRANSFORM], 
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
                
                GLuint OpenglTextureHandle = *(opengl->textures + Data->TextureHandle.Id); 
                
                // NOTE(Momo): If the currently set texture is not same as the currently
                // processed texture, batch draw all instances before the current instance.
                if (CurrentTexture != OpenglTextureHandle) {
                    Opengl_DrawInstances(opengl, 
                                         CurrentTexture, 
                                         InstancesToDrawCount, 
                                         LastDrawnInstanceIndex);
                    LastDrawnInstanceIndex += InstancesToDrawCount;
                    InstancesToDrawCount = 0;
                    CurrentTexture = OpenglTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                opengl->glNamedBufferSubData(opengl->buffers[OPENGL_VBO_COLORS], 
                                             CurrentInstanceIndex * sizeof(v4f),
                                             sizeof(v4f), 
                                             &Data->Colors);
                
                opengl->glNamedBufferSubData(opengl->buffers[OPENGL_VBO_TEXTURE],
                                             CurrentInstanceIndex * sizeof(quad2f),
                                             sizeof(quad2f),
                                             &Data->TextureCoords);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f Transform = transpose(Data->Transform);
                opengl->glNamedBufferSubData(opengl->buffers[OPENGL_VBO_TRANSFORM], 
                                             CurrentInstanceIndex* sizeof(m44f), 
                                             sizeof(m44f), 
                                             &Transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++InstancesToDrawCount;
                ++CurrentInstanceIndex;
                
            } break;
        }
    }
    
    Opengl_DrawInstances(opengl, CurrentTexture, InstancesToDrawCount, LastDrawnInstanceIndex);
    
}



#endif
