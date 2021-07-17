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


struct Renderer_Texture_Handle {
    b8 success;
    u32 id;
};


enum Renderer_Command_Type {
    RENDERER_COMMAND_CLEAR_COLOR,
    RENDERER_COMMAND_SET_BASIS,
    RENDERER_COMMAND_DRAW_TEXTURED_QUAD,
    RENDERER_COMMAND_DRAW_QUAD,
    RENDERER_COMMAND_TYPE_SET_DESIGN_RESOLUTION,
};

struct renderer_command_clear_color {
    c4f Colors;
};

struct renderer_command_set_basis {
    m44f Basis;
};

struct renderer_command_draw_textured_quad {
    Renderer_Texture_Handle TextureHandle;
    c4f Colors;
    m44f Transform;
    quad2f TextureCoords; 
};


struct renderer_command_draw_quad {
    c4f Colors;
    m44f Transform;
};

struct renderer_command_set_design_resolution {
    u32 Width;
    u32 Height;
};

static inline aabb2u 
Renderer_CalcRenderRegion(u32 WindowWidth, 
                          u32 WindowHeight, 
                          u32 RenderWidth, 
                          u32 RenderHeight) 
{
    if ( RenderWidth == 0 || RenderHeight == 0 || WindowWidth == 0 || WindowHeight == 0) {
        return {};
    }
    aabb2u Ret = {};
    
    f32 OptimalWindowWidth = (f32)WindowHeight * ((f32)RenderWidth / (f32)RenderHeight);
    f32 OptimalWindowHeight = (f32)WindowWidth * ((f32)RenderHeight / (f32)RenderWidth);
    
    if (OptimalWindowWidth > (f32)WindowWidth) {
        // NOTE(Momo): Width has priority - top and bottom bars
        Ret.min.x = 0;
        Ret.max.x = WindowWidth;
        
        f32 EmptyHeight = (f32)WindowHeight - OptimalWindowHeight;
        
        Ret.min.y = (u32)(EmptyHeight * 0.5f);
        Ret.max.y = Ret.min.y + (u32)OptimalWindowHeight;
    }
    else {
        // NOTE(Momo): Height has priority - left and right bars
        Ret.min.y = 0;
        Ret.max.y = WindowHeight;
        
        
        f32 EmptyWidth = (f32)WindowWidth - OptimalWindowWidth;
        
        Ret.min.x = (u32)(EmptyWidth * 0.5f);
        Ret.max.x = Ret.min.x + (u32)OptimalWindowWidth;
    }
    
    return Ret;
}



static inline void
Renderer_SetBasis(Mailbox* commands, m44f basis) {
    auto* Data = 
        commands->push_struct<renderer_command_set_basis>(RENDERER_COMMAND_SET_BASIS);
    Data->Basis = basis;
}

static inline void
Renderer_SetOrthoCamera(Mailbox* commands, 
                        v3f position,
                        aabb3f frustum)   
{
    auto* Data = commands->push_struct<renderer_command_set_basis>(RENDERER_COMMAND_SET_BASIS);
    
    auto P  = m44f::create_orthographic(frustum.min.x,  
                                        frustum.max.x, 
                                        frustum.min.y, 
                                        frustum.max.y,
                                        frustum.min.z, 
                                        frustum.max.z);
    
    m44f V = m44f::create_translation(-position.x, -position.y, -position.z);
    Data->Basis = P * V;
}

static inline void
Renderer_ClearColor(Mailbox* commands, c4f colors) {
    auto* Data = commands->push_struct<renderer_command_clear_color>(RENDERER_COMMAND_CLEAR_COLOR);
    Data->Colors = colors;
}



static inline void
Renderer_DrawTexturedQuad(Mailbox* commands, 
                          c4f colors, 
                          m44f transform, 
                          Renderer_Texture_Handle texture_handle,
                          quad2f texture_coords = Quad2f_CreateDefaultUV())  

{
    auto* Data = commands->push_struct<renderer_command_draw_textured_quad>(RENDERER_COMMAND_DRAW_TEXTURED_QUAD);
    
    Data->Colors = colors;
    Data->Transform = transform;
    Data->TextureHandle = texture_handle;
    Data->TextureCoords = texture_coords;
}

static inline void
Renderer_DrawQuad(Mailbox* commands, 
                  c4f colors, 
                  m44f transform) 
{
    auto* Data = commands->push_struct<renderer_command_draw_quad>(RENDERER_COMMAND_DRAW_QUAD);
    Data->Colors = colors;
    Data->Transform = transform;
}

static inline void 
Renderer_DrawLine2f(Mailbox* commands, 
                    line2f Line,
                    f32 Thickness,
                    c4f Colors,
                    f32 PosZ) 
{
    // NOTE(Momo): Min.Y needs to be lower than Max.y
    if (Line.min.y > Line.max.y) {
        SWAP(Line.min, Line.max);
    }
    
    v2f LineVector = sub(Line.max, Line.min);
    f32 LineLength = length(LineVector);
    v2f LineMiddle = midpoint(Line.max, Line.min);
    
    v2f XAxis = { 1.f, 0.f };
    f32 Angle = angle_between(LineVector, XAxis);
    
    //TODO: Change line3f
    m44f T = m44f::create_translation(LineMiddle.x, LineMiddle.y, PosZ);
    m44f R = m44f::create_rotation_z(Angle);
    m44f S = m44f::create_scale(LineLength, Thickness, 1.f) ;
    
    Renderer_DrawQuad(commands, Colors, T*R*S);
}

static inline void
Renderer_DrawCircle2f(Mailbox* commands,
                      circle2f Circle,
                      f32 Thickness, 
                      u32 LineCount,
                      c4f Color,
                      f32 PosZ) 
{
    // NOTE(Momo): Essentially a bunch of lines
    // We can't really have a surface with less than 3 lines
    ASSERT(LineCount >= 3);
    f32 AngleIncrement = TAU / LineCount;
    v2f Pt1 = { 0.f, Circle.radius }; 
    v2f Pt2 = rotate(Pt1, AngleIncrement);
    
    for (u32 I = 0; I < LineCount; ++I) {
        v2f LinePt1 = Pt1 + Circle.origin;
        v2f LinePt2 = Pt2 + Circle.origin;
        line2f Line = line2f::create(LinePt1, LinePt2);
        Renderer_DrawLine2f(commands, 
                            Line,
                            Thickness,
                            Color,
                            PosZ);
        
        Pt1 = Pt2;
        Pt2 = rotate(Pt1, AngleIncrement);
        
    }
}

static inline void 
Renderer_DrawAabb2f(Mailbox* commands, 
                    aabb2f Aabb,
                    f32 Thickness,
                    c4f Colors,
                    f32 PosZ ) 
{
    //Bottom
    Renderer_DrawLine2f(commands, 
                        line2f::create(Aabb.min.x, 
                                       Aabb.min.y,  
                                       Aabb.max.x, 
                                       Aabb.min.y),
                        Thickness, 
                        Colors,
                        PosZ);
    // Left
    Renderer_DrawLine2f(commands, 
                        line2f::create(Aabb.min.x,
                                       Aabb.min.y,
                                       Aabb.min.x,
                                       Aabb.max.y),  
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Top
    Renderer_DrawLine2f(commands, 
                        line2f::create(Aabb.min.x,
                                       Aabb.max.y,
                                       Aabb.max.x,
                                       Aabb.max.y), 
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Right 
    Renderer_DrawLine2f(commands, 
                        line2f::create(Aabb.max.x,
                                       Aabb.min.y,
                                       Aabb.max.x,
                                       Aabb.max.y),  
                        Thickness, 
                        Colors,
                        PosZ);
}

static inline void 
Renderer_SetDesignResolution(Mailbox* Commands, 
                             u32 Width, 
                             u32 Height)  
{
    auto* Data = Commands->push_struct<renderer_command_set_design_resolution>(RENDERER_COMMAND_TYPE_SET_DESIGN_RESOLUTION);
    Data->Width = Width;
    Data->Height = Height;
}

#endif //GAME_RENDERER_H
