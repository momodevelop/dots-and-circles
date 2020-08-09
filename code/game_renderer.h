/* NOTE(Momo): 
Ground rules about this renderer.
- This is a 2D renderer in 3D space. 
- Right-handed coordinate system: +Y is up, +Z is towards
- Only one model is supported: A quad that can be textured and colored 
- Quad points (and thus UV mapping)  is defined in the an anti-clockwise order, starting from bottom left (to match our coordinate system): 
0. bottom left
1. bottom right
2. top right
3. top left 
- Indices layout 2 triangles in the following fashion:
// ---
// |/|
// ---
- UV origin is bottom left
*/

#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "ryoji_render_commands.h"
#include "ryoji_maths.h"
#include "ryoji_asset_types.h"

static inline quad2f
UVRect2ToQuad2(rect2f Rect) {
    return {
        Rect.Min.X, Rect.Min.Y, // bottom left  	
        Rect.Max.X, Rect.Min.Y, // bottom right
        Rect.Max.X, Rect.Max.Y, // top right
        Rect.Min.X, Rect.Max.Y, // top left
    };
}

static inline constexpr quad2f StandardQuadUV = {
    0.f, 0.f, // bottom left
    1.0f, 0.f, // bottom right
    1.0f, 1.0f, // top right
    0.0f, 1.0f,  // top left
};

constexpr static f32 QuadModel[] = {
    // position   
    -0.5f, -0.5f, 0.0f,  // bottom left
    0.5f, -0.5f, 0.0f,  // bottom right
    0.5f,  0.5f, 0.0f,  // top right
    -0.5f,  0.5f, 0.0f,   // top left 
};

constexpr static u8 QuadIndices[] = {
    0, 1, 2,
    0, 2, 3,
};

struct render_command_data_clear {
    static constexpr u32 TypeId = __LINE__;
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
PushCommandDrawTexturedQuad(render_commands* Commands, 
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

struct render_command_data_draw_quad {
    static constexpr u32 TypeId = __LINE__;
    c4f Colors;
    m44f Transform;
};


static inline void
PushCommandDrawQuad(render_commands* Commands, 
                    c4f Colors, 
                    m44f Transform) 
{
    using data_t = render_command_data_draw_quad;
    auto* Data = PushCommand<data_t>(Commands);
    Data->Colors = Colors;
    Data->Transform = Transform;
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
PushCommandDebugLine(render_commands* Commands, 
                     line2f Line, 
                     f32 Thickness = 1.f,
                     c4f Colors = {0.f, 1.f, 0.f, 1.f}) 
{
    f32 LineLength = Length(Line.Max - Line.Min);
    v2f LineMiddle = Midpoint(Line.Max, Line.Min);
    
    v2f LineVector = Line.Max - Line.Min;
    f32 Angle = AngleBetween(LineVector, { 1.f, 0.f });
    
    // TODO(Momo): change Z to biggest possible float number?
    m44f T = TranslationMatrix(LineMiddle.X, LineMiddle.Y, 100.f);
    m44f R = RotationZMatrix(Angle);
    m44f S = ScaleMatrix(LineLength, Thickness, 1.f) ;
    
    m44f Transform = T*R*S;
    
    PushCommandDrawQuad(Commands, Colors, Transform);
}

static inline void 
PushCommandDebugRect(render_commands* Commands, 
                     rect2f Rect, 
                     f32 Thickness = 1.f,
                     c4f Colors = {0.f, 1.f, 0.f, 1.f}) 
{
    //Bottom
    PushCommandDebugLine(Commands, 
                         { 
                             Rect.Min.X, 
                             Rect.Min.Y,  
                             Rect.Max.X, 
                             Rect.Min.Y,
                         }, Thickness, Colors);
    // Left
    PushCommandDebugLine(Commands, 
                         { 
                             Rect.Min.X,
                             Rect.Min.Y,
                             Rect.Min.X,
                             Rect.Max.Y,
                         }, Thickness, Colors);
    
    //Top
    PushCommandDebugLine(Commands, 
                         { 
                             Rect.Min.X,
                             Rect.Max.Y,
                             Rect.Max.X,
                             Rect.Max.Y,
                         }, Thickness, Colors);
    
    //Right 
    PushCommandDebugLine(Commands, 
                         { 
                             Rect.Max.X,
                             Rect.Min.Y,
                             Rect.Max.X,
                             Rect.Max.Y,
                         }, Thickness, Colors);
}
#endif

#endif //GAME_RENDERER_H
