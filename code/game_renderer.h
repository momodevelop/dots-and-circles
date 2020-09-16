/* NOTE(Momo): 
Ground rules about this renderer.
- This is a 2D renderer in 3D space. 
- Right-handed coordinate system: +Y is up, +Z is towards
- Only one model is supported: A quad that can be textured and colored 
- UV origin is from top left. Points given for UV to map to the quad is given in this order:
>> 0. Top left
>> 1. Top right
>> 2. Bottom right
>> 3. Bottom left

- Indices layout 2 triangles in the following fashion:
* ---
 * |/|
 * ---
*/
#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "ryoji_commands.h"
#include "ryoji_maths.h"

struct render_command_clear_color {
    static constexpr u32 TypeId = __LINE__;
    v4f Colors;
};

struct render_command_set_basis {
    static constexpr u32 TypeId = __LINE__;
    m44f Basis;
};

struct render_command_draw_textured_quad {
    static constexpr u32 TypeId = __LINE__;
    u32 TextureHandle;
    v4f Colors;
    m44f Transform;
    quad2f TextureCoords; 
};


struct render_command_draw_quad {
    static constexpr u32 TypeId = __LINE__;
    v4f Colors;
    m44f Transform;
};



struct render_command_link_texture {
    static constexpr u32 TypeId = __LINE__;
    u32 Width;
    u32 Height;
    void* Pixels;
    
    // TODO(Momo): Channels
    // u32 Channels;
    u32 TextureHandle;
};

struct render_command_set_design_resolution {
    static constexpr u32 TypeId = __LINE__;
    u32 Width;
    u32 Height;
};


#include "game_renderer.cc"

#endif //GAME_RENDERER_H
