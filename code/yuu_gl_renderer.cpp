#ifndef __YUU_GLAD_RENDERER__
#define __YUU_GLAD_RENDERER__

#include "yuu_gl.cpp"
// NOTE(Momo): For now, we are only a sprite manager


// A sprite is only a textured quad, so we only need one vao.
struct GLRenderer {
    enum VBOTypes {
        VBO_MODEL,     
        VBO_INDICES,
        VBO_TRANSFORMS, 
        VBO_COLORS,
        VBO_TEXTURES,
        VBO_MAX
    };
    GLuint vbos[VBO_MAX];
    GLuint vao;
    usize maxRenderables;
};

pure void Init(GLRenderer* r, usize maxRenderables) {
    u8 quadIndices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    
    f32 quadModel[] = {
        // position   
        1.f,  1.f, 0.0f,  // top right
        1.f, -1.f, 0.0f,  // bottom right
        -1.f, -1.f, 0.0f,  // bottom left
        -1.f,  1.f, 0.0f   // top left 
    };
    
    r->maxRenderables = maxRenderables;
    
    // Setup VBOs
    glCreateBuffers(VBO_MAX, r->vbo);
    glCreateNamedBufferStorage(vbo[VBO_MODEL], sizeof(quadModel), quadModel, 0);
    glCreateNamedBufferStorage(vbo[VBO_INDICES], sizeof(quadIndices), quadIndices, 0);
    glCreateNamedBufferStorage(vbo[VBO_COLORS], sizeof(GLfloat)*4*maxRenderables, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glCreateNamedBufferStorage(vbo[VBO_TRANSFORMS], sizeof(GLfloat)*16*maxRenderables, nullptr, GL_DYNAMIC_STORAGE_BIT);
    
    // TODO(Momo): Textures
    //glCreateNamedBufferStoage(vbo[VBO_TEXTURES], sizeo
    
    // Setup VAO
    glCreateVertexArrays(1, &r->&vao);
}

pure void AddModel(GLModelManager* mgr, f32* vertices, u8* indices) {
    
}


