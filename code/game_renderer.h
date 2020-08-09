/* NOTE(Momo): 
Ground rules about this renderer.
- This is a 2D renderer in 3D space. 
- Right-handed coordinate system: +Y is up, +Z is towards
- Only one model is supported: A quad that can be textured and colored 
- Quad points (and thus UV mapping)  is defined in the an anti-clockwise order, starting from bottom left (to match our coordinate system): 
1. bottom left
2. bottom right
3. top right
4. top left 
- UV: 
*/

#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "ryoji_render_commands.h"
#include "ryoji_maths.h"

#include "game_assets.h"

static inline constexpr quad2f StandardQuadUV = {
    0.0f, 0.0f,  // top left
    1.0f, 0.0f, // top right
    0.f, 1.f, // bottom left
    1.0f, 1.f, // bottom right
};

constexpr static f32 QuadModel[] = {
    // position   
    -0.5f,  0.5f, 0.0f,   // top left 
    0.5f,  0.5f, 0.0f,  // top right
    -0.5f, -0.5f, 0.0f,  // bottom left
    0.5f, -0.5f, 0.0f,  // bottom right
};

constexpr static u8 QuadIndices[] = {
    0, 1, 2,
    1, 3, 2,
};

struct render_command_data_clear {
    static constexpr u32 TypeId = 0;
    c4f Colors;
};

static inline void
PushCommandClear(render_commands* Commands, c4f Colors) {
    using data_t = render_command_data_clear;
    auto* Data = PushCommand<data_t>(Commands);
    Data->Colors = Colors;
}


struct render_command_data_textured_quad {
    static constexpr u32 TypeId = __LINE__;
    u32 TextureHandle;
    c4f Colors;
    m44f Transform;
    quad2f TextureCoords; 
};


static inline void
PushCommandTexturedQuad(render_commands* Commands, 
                        c4f Colors, 
                        m44f Transform, 
                        u32 TextureHandle,
                        quad2f TextureCoords = StandardQuadUV) 
{
    using data_t = render_command_data_textured_quad;
    auto* Data = PushCommand<data_t>(Commands);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = TextureCoords;
}

struct render_command_data_link_texture {
    static constexpr u32 TypeId = __LINE__;
    bitmap TextureBitmap;
    u32 TextureHandle;
};


static inline void 
PushCommandLinkTexture(render_commands* Commands, 
                       bitmap TextureBitmap, 
                       u32 TextureHandle) {
    using data_t = render_command_data_link_texture;
    auto* Data = PushCommand<data_t>(Commands);
    Data->TextureBitmap = TextureBitmap;
    Data->TextureHandle = TextureHandle;
}

#if INTERNAL
static inline void 
PushCommandDebugLine(render_commands* Commands, line2f Line, f32 Thickness = 1.f) {
    f32 LineLength = Length(Line.Max - Line.Min);
    v2f LineMiddle = Midpoint(Line.Max, Line.Min);
    
    v2f LineVector = Line.Max - Line.Min;
    f32 Angle = AngleBetween(LineVector, { 1.f, 0.f });
    
    // TODO(Momo): change Z to biggest possible float number?
    m44f T = TranslationMatrix(LineMiddle.X, LineMiddle.Y, 100.f);
    m44f R = RotationZMatrix(Angle);
    m44f S = ScaleMatrix(LineLength, Thickness, 1.f) ;
    
    m44f Transform = T*R*S;
    
    quad2f UVCoords = {
        0.0f, 0.0f,
        1.f, 0.f,
        0.f, 1.f,
        1.f, 1.f
    };
    c4f Colors = {
        0.f, 1.f, 0.f, 1.f
    };
    
    PushCommandTexturedQuad(Commands, Colors, Transform, GameBitmapHandle_Blank, UVCoords);
}
#endif

#endif //GAME_RENDERER_H
