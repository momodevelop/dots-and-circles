/* NOTE(Momo): 
Ground rules about this renderer.
- This is a 2D renderer in 3D space. 
- Right-handed coordinate system: +Y is up, +Z is towards you
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
#ifndef RENDERER_H
#define RENDERER_H

#include "mm_mailbox.h"
#include "mm_maths.h"
#include "mm_colors.h"

// "Inherited" by all renderers
struct renderer {
    b32 IsInitialized;
};

struct renderer_texture_handle {
    u32 Id;
};

struct renderer_command_clear_color {
    static constexpr u32 TypeId = __LINE__;
    c4f Colors;
};

struct renderer_command_set_basis {
    static constexpr u32 TypeId = __LINE__;
    m44f Basis;
};

struct renderer_command_draw_textured_quad {
    static constexpr u32 TypeId = __LINE__;
    
    renderer_texture_handle TextureHandle;
    c4f Colors;
    m44f Transform;
    quad2f TextureCoords; 
};


struct renderer_command_draw_quad {
    static constexpr u32 TypeId = __LINE__;
    c4f Colors;
    m44f Transform;
};

struct renderer_command_set_design_resolution {
    static constexpr u32 TypeId = __LINE__;
    u32 Width;
    u32 Height;
};

static inline aabb2u 
GetRenderRegion(u32 WindowWidth, 
                u32 WindowHeight, 
                u32 RenderWidth, 
                u32 RenderHeight) 
{
    Assert(RenderWidth > 0 && RenderHeight > 0 && WindowWidth > 0 && WindowHeight > 0);
    aabb2u Ret = {};
    
    f32 OptimalWindowWidth = (f32)WindowHeight * ((f32)RenderWidth / (f32)RenderHeight);
    f32 OptimalWindowHeight = (f32)WindowWidth * ((f32)RenderHeight / (f32)RenderWidth);
    
    if (OptimalWindowWidth > (f32)WindowWidth) {
        // NOTE(Momo): Width has priority - top and bottom bars
        Ret.Min.X = 0;
        Ret.Max.X = WindowWidth;
        
        f32 EmptyHeight = (f32)WindowHeight - OptimalWindowHeight;
        
        Ret.Min.Y = (u32)(EmptyHeight * 0.5f);
        Ret.Max.Y = Ret.Min.Y + (u32)OptimalWindowHeight;
    }
    else {
        // NOTE(Momo): Height has priority - left and right bars
        Ret.Min.Y = 0;
        Ret.Max.Y = WindowHeight;
        
        
        f32 EmptyWidth = (f32)WindowWidth - OptimalWindowWidth;
        
        Ret.Min.X = (u32)(EmptyWidth * 0.5f);
        Ret.Max.X = Ret.Min.X + (u32)OptimalWindowWidth;
    }
    
    return Ret;
}



static inline void
PushSetBasis(mailbox* Commands, m44f Basis) {
    using data_t = renderer_command_set_basis;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    Data->Basis = Basis;
}

static inline void
PushOrthoCamera(mailbox* Commands, 
                v3f Position,
                aabb3f Frustum)   
{
    using data_t = renderer_command_set_basis;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    
    auto P  = M44fOrthographic(-1.f, 1.f,
                               -1.f, 1.f,
                               -1.f, 1.f,
                               Frustum.Min.X,  
                               Frustum.Max.X, 
                               Frustum.Min.Y, 
                               Frustum.Max.Y,
                               Frustum.Min.Z, 
                               Frustum.Max.Z,
                               true);
    
    m44f V = M44fTranslation(-Position.X, -Position.Y, -Position.Z);
    Data->Basis = P*V;
}

static inline void
PushClearColor(mailbox* Commands, c4f Colors) {
    using data_t = renderer_command_clear_color;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    Data->Colors = Colors;
}



static inline void
PushDrawTexturedQuad(mailbox* Commands, 
                     c4f Colors, 
                     m44f Transform, 
                     renderer_texture_handle TextureHandle,
                     quad2f TextureCoords = {
                         0.0f, 1.0f,  // top left
                         1.0f, 1.0f, // top right
                         1.0f, 0.f, // bottom right
                         0.f, 0.f, // bottom left
                     }) 
{
    using data_t = renderer_command_draw_textured_quad;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = TextureCoords;
}

static inline void
PushDrawTexturedQuad(mailbox* Commands,
                     c4f Colors, 
                     m44f Transform, 
                     renderer_texture_handle TextureHandle,
                     aabb2f TextureCoords) 
{
    using data_t = renderer_command_draw_textured_quad;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = Quad2f(TextureCoords);
}


static inline void
PushDrawQuad(mailbox* Commands, 
             c4f Colors, 
             m44f Transform) 
{
    using data_t = renderer_command_draw_quad;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    Data->Colors = Colors;
    Data->Transform = Transform;
}

static inline void 
PushDrawLine(mailbox* Payload, 
             line2f Line, 
             f32 Thickness,
             c4f Colors) 
{
    // NOTE(Momo): Min.Y needs to be lower than Max.Y
    if (Line.Min.Y > Line.Max.Y) {
        Swap(&Line.Min, &Line.Max);
    }
    
    f32 LineLength = Length(Line.Max - Line.Min);
    v2f LineMiddle = Midpoint(Line.Max, Line.Min);
    
    v2f LineVector = Line.Max - Line.Min;
    f32 Angle = AngleBetween(LineVector, { 1.f, 0.f });
    
    
    m44f T = M44fTranslation(LineMiddle.X, LineMiddle.Y, 100.f);
    m44f R = M44fRotationZ(Angle);
    m44f S = M44fScale(LineLength, Thickness, 1.f) ;
    
    m44f Transform = T*R*S;
    
    PushDrawQuad(Payload, Colors, Transform);
}

static inline void 
PushDrawLineAabb(mailbox* Commands, 
                 aabb2f Aabb,
                 f32 Thickness,
                 c4f Colors) 
{
    //Bottom
    PushDrawLine(Commands, 
                 { 
                     Aabb.Min.X, 
                     Aabb.Min.Y,  
                     Aabb.Max.X, 
                     Aabb.Min.Y,
                 },  Thickness, Colors);
    // Left
    PushDrawLine(Commands, 
                 { 
                     Aabb.Min.X,
                     Aabb.Min.Y,
                     Aabb.Min.X,
                     Aabb.Max.Y,
                 },  Thickness, Colors);
    
    //Top
    PushDrawLine(Commands, 
                 { 
                     Aabb.Min.X,
                     Aabb.Max.Y,
                     Aabb.Max.X,
                     Aabb.Max.Y,
                 }, Thickness, Colors);
    
    //Right 
    PushDrawLine(Commands, 
                 { 
                     Aabb.Max.X,
                     Aabb.Min.Y,
                     Aabb.Max.X,
                     Aabb.Max.Y,
                 },  Thickness, Colors);
}

static inline void 
PushSetDesignResolution(mailbox* Commands, 
                        u32 Width, 
                        u32 Height)  
{
    using data_t = renderer_command_set_design_resolution;
    auto* Data = Push<data_t>(Commands, data_t::TypeId);
    Data->Width = Width;
    Data->Height = Height;
}

#endif //GAME_RENDERER_H
