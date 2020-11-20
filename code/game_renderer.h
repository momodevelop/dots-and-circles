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
#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "mm_commands.h"
#include "mm_maths.h"


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


static inline rect2u 
GetRenderRegion(u32 WindowWidth, 
                u32 WindowHeight, 
                u32 RenderWidth, 
                u32 RenderHeight) {
    Assert(RenderWidth > 0 && RenderHeight > 0 && WindowWidth > 0 && WindowHeight > 0);
    rect2u Ret = {};
    
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
PushCommandSetBasis(mmcmd_commands* Commands, m44f Basis) {
    using data_t = render_command_set_basis;
    auto* Data = mmcmd_Push<data_t>(Commands);
    Data->Basis = Basis;
}

static inline void
PushCommandSetOrthoBasis(mmcmd_commands* Commands, 
                         v3f Origin,
                         v3f Dimensions)   
{
    using data_t = render_command_set_basis;
    auto* Data = mmcmd_Push<data_t>(Commands);
    
    auto P  = Orthographic(-1.f, 1.f,
                                 -1.f, 1.f,
                                 -1.f, 1.f,
                                 -Dimensions.W * 0.5f,  
                                 Dimensions.W * 0.5f, 
                                 -Dimensions.H * 0.5f, 
                                 Dimensions.H* 0.5f,
                                 -Dimensions.D * 0.5f, 
                                 Dimensions.D * 0.5f,
                                 true);
    
    m44f V = Translation(-Origin.X, -Origin.Y, 0.f);
    Data->Basis = P*V;
}

static inline void
PushCommandClearColor(mmcmd_commands* Commands, v4f Colors) {
    using data_t = render_command_clear_color;
    auto* Data = mmcmd_Push<data_t>(Commands);
    Data->Colors = Colors;
}



static inline void
PushCommandDrawTexturedQuad(mmcmd_commands* Commands, 
                            v4f Colors, 
                            m44f Transform, 
                            u32 TextureHandle,
                            quad2f TextureCoords = {
                                0.0f, 1.0f,  // top left
                                1.0f, 1.0f, // top right
                                1.0f, 0.f, // bottom right
                                0.f, 0.f, // bottom left
                                
                            }) 
{
    using data_t = render_command_draw_textured_quad;
    auto* Data = mmcmd_Push<data_t>(Commands);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = TextureCoords;
}

static inline void
PushCommandDrawTexturedQuad(mmcmd_commands* Commands,
                            v4f Colors, 
                            m44f Transform, 
                            u32 TextureHandle,
                            rect2f TextureCoords) 
{
    using data_t = render_command_draw_textured_quad;
    auto* Data = mmcmd_Push<data_t>(Commands);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = Quad2F(TextureCoords);
}


static inline void
PushCommandDrawQuad(mmcmd_commands* Commands, 
                    v4f Colors, 
                    m44f Transform) 
{
    using data_t = render_command_draw_quad;
    auto* Data = mmcmd_Push<data_t>(Commands);
    Data->Colors = Colors;
    Data->Transform = Transform;
}


static inline void 
PushCommandLinkTexture(mmcmd_commands* Commands, 
                       u32 Width,
                       u32 Height,
                       void* Pixels,
                       u32 TextureHandle) {
    using data_t = render_command_link_texture;
    auto* Data = mmcmd_Push<data_t>(Commands);
    Data->Width = Width;
    Data->Height = Height;
    Data->Pixels = Pixels;
    Data->TextureHandle = TextureHandle;
}


static inline void 
PushCommandDrawLine(mmcmd_commands* Payload, 
                    line2f Line, 
                    f32 Thickness = 2.f,
                    v4f Colors = {0.f, 1.f, 0.f, 1.f}) 
{
    // NOTE(Momo): Min.Y needs to be lower than Max.Y
    if (Line.Min.Y > Line.Max.Y) {
        Swap(Line.Min, Line.Max);
    }
    
    f32 LineLength = Length(Line.Max - Line.Min);
    v2f LineMiddle = Midpoint(Line.Max, Line.Min);
    
    v2f LineVector = Line.Max - Line.Min;
    f32 Angle = AngleBetween(LineVector, { 1.f, 0.f });
    
    
    m44f T = Translation(LineMiddle.X, LineMiddle.Y, 100.f);
    m44f R = RotationZ(Angle);
    m44f S = Scale(LineLength, Thickness, 1.f) ;
    
    m44f Transform = T*R*S;
    
    PushCommandDrawQuad(Payload, Colors, Transform);
}

static inline void 
PushCommandDrawLineRect(mmcmd_commands* Commands, 
                        rect2f Rect,
                        f32 Thickness = 1.f,
                        v4f Colors = {0.f, 1.f, 0.f, 1.f}) 
{
    //Bottom
    PushCommandDrawLine(Commands, 
                        { 
                            Rect.Min.X, 
                            Rect.Min.Y,  
                            Rect.Max.X, 
                            Rect.Min.Y,
                        },  Thickness, Colors);
    // Left
    PushCommandDrawLine(Commands, 
                        { 
                            Rect.Min.X,
                            Rect.Min.Y,
                            Rect.Min.X,
                            Rect.Max.Y,
                        },  Thickness, Colors);
    
    //Top
    PushCommandDrawLine(Commands, 
                        { 
                            Rect.Min.X,
                            Rect.Max.Y,
                            Rect.Max.X,
                            Rect.Max.Y,
                        }, Thickness, Colors);
    
    //Right 
    PushCommandDrawLine(Commands, 
                        { 
                            Rect.Max.X,
                            Rect.Min.Y,
                            Rect.Max.X,
                            Rect.Max.Y,
                        },  Thickness, Colors);
}

static inline void 
PushCommandSetDesignResolution(mmcmd_commands* Commands, u32 Width, u32 Height)  
{
    using data_t = render_command_set_design_resolution;
    auto* Data = mmcmd_Push<data_t>(Commands);
    Data->Width = Width;
    Data->Height = Height;
}

#endif //GAME_RENDERER_H
