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
typedef smi GLsizeiptr; 
typedef smi GLintptr;
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


// NOTE(Momo): buffers
enum {
    OPENGL_VBO_MODEL,
    OPENGL_VBO_INDICES,
    OPENGL_VBO_COLORS,
    OPENGL_VBO_TEXTURE,
    OPENGL_VBO_TRANSFORM,
    OPENGL_VBO_MAX
};


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
    
    constexpr static const char* vertex_shader = R"###(
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
    
    constexpr static const char* fragment_shader = R"###(
    #version 450 core
    out vec4 fragColor;
    in vec4 mColor;
    in vec2 mTexCoord;
    uniform sampler2D uTexture;
    
    void main(void) {
        fragColor = texture(uTexture, mTexCoord) * mColor; 
    })###";
    
    
    // Stuff to work with game
    constexpr static inline f32 quad_model[] = {
        -0.5f, -0.5f, 0.0f,  // bottom left
        0.5f, -0.5f, 0.0f,  // bottom right
        0.5f,  0.5f, 0.0f,  // top right
        -0.5f,  0.5f, 0.0f,   // top left 
    };
    
    constexpr static inline u8 quad_indices[] = {
        0, 1, 2,
        0, 2, 3,
    };
    
    constexpr static inline f32 quad_uv[] = {
        0.0f, 1.0f,  // top left
        1.0f, 1.0f, // top right
        1.0f, 0.f, // bottom right
        0.f, 0.f, // bottom left
    };
    
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
    
    
    inline void attach_shader(u32 program, u32 type, char* code);
    inline void align_viewport();
    inline void add_predefined_textures();
    inline void draw_instances(GLuint texture, u32 instances_to_draw, u32 index_to_draw_from);
    
    // public
    inline b8 init(Arena* arena, u32 w, u32 h);
    inline void resize(u32 w, u32 h);
    inline void clear_textures();
    inline Renderer_Texture_Handle add_texture(u32 w, u32 h, void* pixels);
    inline void render(Mailbox* commands); 
    
};

void 
Opengl::attach_shader(u32 program, u32 type, char* Code) {
    GLuint shader_handle = this->glCreateShader(type);
    this->glShaderSource(shader_handle, 1, &Code, NULL);
    this->glCompileShader(shader_handle);
    this->glAttachShader(program, shader_handle);
    this->glDeleteShader(shader_handle);
}

void 
Opengl::align_viewport() 
{
    aabb2u region = Renderer_CalcRenderRegion(this->window_dimensions.w, 
                                              this->window_dimensions.h, 
                                              this->design_dimensions.w, 
                                              this->design_dimensions.h);
    
    u32 x, y, w, h;
    x = region.min.x;
    y = region.min.y;
    w = width(region);
    h = height(region);
    
    this->glViewport(x, y, w, h);
    this->glScissor(x, y, w, h);
    
    // NOTE(Momo): Cache this to make calculations 
    // for window-space to render-space more CPU friendly.
    this->render_region = region;
}




void 
Opengl::resize(u32 w, u32 h) 
{
    this->window_dimensions.w = w;
    this->window_dimensions.h = h;
    align_viewport();
}

void 
Opengl::add_predefined_textures() {
    struct Pixel { u8 e[4]; };
    
    
    // NOTE(Momo): Dummy texture setup
    {
        Pixel pixels[4] = {
            { 125, 125, 125, 255 },
            { 255, 255, 255, 255 },
            { 255, 255, 255, 255 },
            { 125, 125, 125, 255 },
        };
        
        GLuint dummy_texture;
        this->glCreateTextures(GL_TEXTURE_2D, 1, &dummy_texture);
        this->glTextureStorage2D(dummy_texture, 1, GL_RGBA8, 2, 2);
        this->glTextureSubImage2D(dummy_texture, 
                                  0, 0, 0, 
                                  2, 2, 
                                  GL_RGBA, 
                                  GL_UNSIGNED_BYTE, 
                                  &pixels);
        List_Push_Item(&textures, dummy_texture);
    }
    
    // NOTE(Momo): Blank texture setup
    {
        Pixel pixels = { 255, 255, 255, 255 };
        GLuint blank_texture;
        this->glCreateTextures(GL_TEXTURE_2D, 1, &blank_texture);
        this->glTextureStorage2D(blank_texture, 1, GL_RGBA8, 1, 1);
        this->glTextureSubImage2D(blank_texture, 
                                  0, 0, 0, 
                                  1, 1, 
                                  GL_RGBA, GL_UNSIGNED_BYTE, 
                                  &pixels);
        List_Push_Item(&textures, blank_texture);
    }
    
    
}

b8
Opengl::init(Arena* arena,
             u32 w, u32 h) 
{
    List_Alloc(&this->textures, arena, OPENGL_MAX_TEXTURES);
    this->design_dimensions = { w, h };
    this->window_dimensions = { w, h };
    
    this->glEnable(GL_DEPTH_TEST);
    this->glEnable(GL_SCISSOR_TEST);
    
    this->glClear(GL_C4f_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->glClearColor(0.f, 0.f, 0.f, 0.f);
    
    this->align_viewport();
    
    
    // NOTE(Momo): Setup VBO
    this->glCreateBuffers(OPENGL_VBO_MAX, this->buffers);
    this->glNamedBufferStorage(this->buffers[OPENGL_VBO_MODEL], 
                               sizeof(quad_model), 
                               quad_model, 
                               0);
    
    this->glNamedBufferStorage(this->buffers[OPENGL_VBO_INDICES], 
                               sizeof(quad_indices), 
                               quad_indices, 
                               0);
    
    this->glNamedBufferStorage(this->buffers[OPENGL_VBO_TEXTURE], 
                               sizeof(v2f) * 4 * OPENGL_MAX_ENTITIES, 
                               nullptr, 
                               GL_DYNAMIC_STORAGE_BIT);
    
    this->glNamedBufferStorage(this->buffers[OPENGL_VBO_COLORS], 
                               sizeof(v4f) * OPENGL_MAX_ENTITIES, 
                               nullptr, 
                               GL_DYNAMIC_STORAGE_BIT);
    
    this->glNamedBufferStorage(this->buffers[OPENGL_VBO_TRANSFORM], 
                               sizeof(m44f) * OPENGL_MAX_ENTITIES, 
                               nullptr, 
                               GL_DYNAMIC_STORAGE_BIT);
    
    
    // NOTE(Momo): Setup VAO
    this->glCreateVertexArrays(1, &this->model);
    this->glVertexArrayVertexBuffer(this->model, 
                                    OPENGL_VAO_BIND_MODEL, 
                                    this->buffers[OPENGL_VBO_MODEL], 
                                    0, 
                                    sizeof(f32)*3);
    
    this->glVertexArrayVertexBuffer(this->model, 
                                    OPENGL_VAO_BIND_TEXTURE, 
                                    this->buffers[OPENGL_VBO_TEXTURE], 
                                    0, 
                                    sizeof(f32) * 8);
    
    this->glVertexArrayVertexBuffer(this->model, 
                                    OPENGL_VAO_BIND_COLORS, 
                                    this->buffers[OPENGL_VBO_COLORS],  
                                    0, 
                                    sizeof(v4f));
    
    this->glVertexArrayVertexBuffer(this->model, 
                                    OPENGL_VAO_BIND_TRANSFORM, 
                                    this->buffers[OPENGL_VBO_TRANSFORM], 
                                    0, 
                                    sizeof(m44f));
    
    // NOTE(Momo): Setup Attributes
    // aModelVtx
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_MODEL); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_MODEL, 
                                    3, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    0);
    
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_MODEL, 
                                     OPENGL_VAO_BIND_MODEL);
    
    // aColor
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_COLORS); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_COLORS, 
                                    4, 
                                    GL_FLOAT, GL_FALSE, 0);
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_COLORS, 
                                     OPENGL_VAO_BIND_COLORS);
    
    this->glVertexArrayBindingDivisor(this->model, OPENGL_VAO_BIND_COLORS, 1); 
    
    // aTexCoord
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TEXTURE_1); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TEXTURE_1, 
                                    2, 
                                    GL_FLOAT, 
                                    GL_FALSE,
                                    sizeof(v2f) * 0);
    
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TEXTURE_2); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TEXTURE_2, 
                                    2, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(v2f) * 1);
    
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TEXTURE_3); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TEXTURE_3, 
                                    2, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(v2f) * 2);
    
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TEXTURE_4); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TEXTURE_4, 
                                    2, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(v2f) * 3);
    
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TEXTURE_1, 
                                     OPENGL_VAO_BIND_TEXTURE);
    
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TEXTURE_2, 
                                     OPENGL_VAO_BIND_TEXTURE);
    
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TEXTURE_3, 
                                     OPENGL_VAO_BIND_TEXTURE);
    
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TEXTURE_4, 
                                     OPENGL_VAO_BIND_TEXTURE);
    
    this->glVertexArrayBindingDivisor(this->model, 
                                      OPENGL_VAO_BIND_TEXTURE, 
                                      1); 
    
    
    // aTransform
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TRANSFORM_1); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TRANSFORM_1, 
                                    4, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(f32) * 0 * 4);
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TRANSFORM_2);
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TRANSFORM_2, 
                                    4, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(f32) * 1 * 4);
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TRANSFORM_3); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TRANSFORM_3, 
                                    4, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(f32) * 2 * 4);
    this->glEnableVertexArrayAttrib(this->model, OPENGL_ATB_TRANSFORM_4); 
    this->glVertexArrayAttribFormat(this->model, 
                                    OPENGL_ATB_TRANSFORM_4,
                                    4, 
                                    GL_FLOAT, 
                                    GL_FALSE, 
                                    sizeof(f32) * 3 * 4);
    
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TRANSFORM_1, 
                                     OPENGL_VAO_BIND_TRANSFORM);
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TRANSFORM_2, 
                                     OPENGL_VAO_BIND_TRANSFORM);
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TRANSFORM_3, 
                                     OPENGL_VAO_BIND_TRANSFORM);
    this->glVertexArrayAttribBinding(this->model, 
                                     OPENGL_ATB_TRANSFORM_4, 
                                     OPENGL_VAO_BIND_TRANSFORM);
    
    this->glVertexArrayBindingDivisor(this->model, 
                                      OPENGL_VAO_BIND_TRANSFORM, 
                                      1); 
    
    // NOTE(Momo): alpha blend
    this->glEnable(GL_BLEND);
    this->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // NOTE(Momo): Setup indices
    this->glVertexArrayElementBuffer(this->model, 
                                     this->buffers[OPENGL_VBO_INDICES]);
    
    
    
    // NOTE(Momo): Setup shader Program
    this->shader = this->glCreateProgram();
    this->attach_shader(this->shader, 
                        GL_VERTEX_SHADER, 
                        (char*)this->vertex_shader);
    this->attach_shader(this->shader, 
                        GL_FRAGMENT_SHADER, 
                        (char*)this->fragment_shader);
    
    this->glLinkProgram(this->shader);
    
    GLint Result;
    this->glGetProgramiv(this->shader, GL_LINK_STATUS, &Result);
    if (Result != GL_TRUE) {
        char msg[KIBIBYTE];
        this->glGetProgramInfoLog(this->shader, KIBIBYTE, nullptr, msg);
        // TODO(Momo): Log?
        return false;
    }
    this->add_predefined_textures();
    this->is_initialized = true;
    return true;
}


void 
Opengl::draw_instances(GLuint texture, 
                       u32 instances_to_draw, 
                       u32 index_to_draw_from) 
{
    ASSERT(instances_to_draw + index_to_draw_from < OPENGL_MAX_ENTITIES);
    if (instances_to_draw > 0) {
        this->glBindTexture(GL_TEXTURE_2D, texture);
        this->glTexParameteri(GL_TEXTURE_2D, 
                              GL_TEXTURE_MIN_FILTER, 
                              GL_NEAREST);
        this->glTexParameteri(GL_TEXTURE_2D, 
                              GL_TEXTURE_MAG_FILTER, 
                              GL_NEAREST);
        this->glEnable(GL_BLEND);
        this->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        this->glBindVertexArray(this->model);
        this->glUseProgram(this->shader);
        
        this->glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 
                                                  6, 
                                                  GL_UNSIGNED_BYTE, 
                                                  nullptr, 
                                                  instances_to_draw,
                                                  index_to_draw_from);
    }
}


Renderer_Texture_Handle
Opengl::add_texture(u32 width,
                    u32 height,
                    void* pixels) 
{
    Renderer_Texture_Handle ret = {};
    
    if (List_Remaining(&this->textures) == 0) {
        ret.success = false;
        ret.id = 0;
        return ret;
    }
    
    
    GLuint entry;
    
    this->glCreateTextures(GL_TEXTURE_2D, 
                           1, 
                           &entry);
    
    this->glTextureStorage2D(entry, 
                             1, 
                             GL_RGBA8, 
                             width, 
                             height);
    
    this->glTextureSubImage2D(entry, 
                              0, 
                              0, 
                              0, 
                              width, 
                              height, 
                              GL_RGBA, 
                              GL_UNSIGNED_BYTE, 
                              pixels);
    
    ret.id = (u32)this->textures.count;
    ret.success = true;
    List_Push_Item(&this->textures, entry);
    return ret;
}

void
Opengl::clear_textures() {
    this->glDeleteTextures((s32)this->textures.count, 
                           this->textures.data);
    List_Clear(&this->textures);
    this->add_predefined_textures();
}


void
Opengl::render(Mailbox* commands) 
{
    // TODO(Momo): Better way to do this without binding texture first?
    GLuint current_texture = 0;
    u32 instances_to_draw = 0;
    u32 last_drawn_instance_index = 0;
    u32 current_instance_index = 0;
    
    this->glClear(GL_C4f_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (u32 i = 0; i < commands->entry_count; ++i) {
        Mailbox_Entry_Header* Entry = commands->get_entry(i);
        
        switch(Entry->type) {
            case RENDERER_COMMAND_TYPE_SET_DESIGN_RESOLUTION: {
                auto* data = (renderer_command_set_design_resolution*)
                    commands->get_entry_data(Entry);
                
                this->design_dimensions.w = data->Width;
                this->design_dimensions.h = data->Height;
                this->align_viewport();
            } break;
            case RENDERER_COMMAND_SET_BASIS: {
                auto* data = (renderer_command_set_basis*)
                    commands->get_entry_data(Entry);
                
                this->draw_instances(current_texture, 
                                     instances_to_draw, 
                                     last_drawn_instance_index);
                last_drawn_instance_index += instances_to_draw;
                instances_to_draw = 0;
                
                auto Result = transpose(data->Basis);
                GLint uProjectionLoc = this->glGetUniformLocation(this->shader,
                                                                  "uProjection");
                
                this->glProgramUniformMatrix4fv(this->shader, 
                                                uProjectionLoc, 
                                                1, 
                                                GL_FALSE, 
                                                (const GLfloat*)&Result);
            } break;
            case RENDERER_COMMAND_CLEAR_COLOR: {
                auto* data = (renderer_command_clear_color*)
                    commands->get_entry_data(Entry);
                this->glClearColor(data->Colors.r, 
                                   data->Colors.g, 
                                   data->Colors.b, 
                                   data->Colors.a);
            } break;
            case RENDERER_COMMAND_DRAW_QUAD: {
                auto* data = (renderer_command_draw_quad*)
                    commands->get_entry_data(Entry);
                
                GLuint OpenglTextureHandle = *(this->textures + OPENGL_PREDEF_TEXTURE_BLANK);
                
                // NOTE(Momo): If the currently set texture is not same as the 
                // currently processed texture, batch draw all instances before 
                // the current instance.
                if (current_texture != OpenglTextureHandle) {
                    this->draw_instances(current_texture, 
                                         instances_to_draw, 
                                         last_drawn_instance_index);
                    last_drawn_instance_index += instances_to_draw;
                    instances_to_draw = 0;
                    current_texture = OpenglTextureHandle;
                }
                
                // NOTE(Momo): Update the current instance values
                this->glNamedBufferSubData(this->buffers[OPENGL_VBO_COLORS], 
                                           current_instance_index * sizeof(v4f),
                                           sizeof(v4f), 
                                           &data->Colors);
                
                this->glNamedBufferSubData(this->buffers[OPENGL_VBO_TEXTURE],
                                           current_instance_index * sizeof(quad2f),
                                           sizeof(quad_uv),
                                           &quad_uv);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f transform = transpose(data->Transform);
                this->glNamedBufferSubData(this->buffers[OPENGL_VBO_TRANSFORM], 
                                           current_instance_index* sizeof(m44f), 
                                           sizeof(m44f), 
                                           &transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++instances_to_draw;
                ++current_instance_index;
            } break;
            case RENDERER_COMMAND_DRAW_TEXTURED_QUAD: {
                auto* data = (renderer_command_draw_textured_quad*)
                    commands->get_entry_data(Entry);
                
                GLuint texture_handle = *(this->textures + data->TextureHandle.id); 
                
                // NOTE(Momo): If the currently set texture is not same as the currently
                // processed texture, batch draw all instances before the current instance.
                if (current_texture != texture_handle) {
                    this->draw_instances(current_texture, 
                                         instances_to_draw, 
                                         last_drawn_instance_index);
                    last_drawn_instance_index += instances_to_draw;
                    instances_to_draw = 0;
                    current_texture = texture_handle;
                }
                
                // NOTE(Momo): Update the current instance values
                this->glNamedBufferSubData(this->buffers[OPENGL_VBO_COLORS], 
                                           current_instance_index * sizeof(v4f),
                                           sizeof(v4f), 
                                           &data->Colors);
                
                this->glNamedBufferSubData(this->buffers[OPENGL_VBO_TEXTURE],
                                           current_instance_index * sizeof(quad2f),
                                           sizeof(quad2f),
                                           &data->TextureCoords);
                
                // NOTE(Momo): Transpose; game is row-major
                m44f transform = transpose(data->Transform);
                this->glNamedBufferSubData(this->buffers[OPENGL_VBO_TRANSFORM], 
                                           current_instance_index* sizeof(m44f), 
                                           sizeof(m44f), 
                                           &transform);
                
                // NOTE(Momo): Update Bookkeeping
                ++instances_to_draw;
                ++current_instance_index;
                
            } break;
        }
    }
    
    this->draw_instances(current_texture, instances_to_draw, last_drawn_instance_index);
    
}



#endif
