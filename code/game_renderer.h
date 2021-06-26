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


struct renderer_texture_handle {
    b8 Success;
    u32 Id;
};


enum renderer_command_type {
    RendererCommandType_ClearColor,
    RendererCommandType_SetBasis,
    RendererCommandType_DrawTexturedQuad,
    RendererCommandType_DrawQuad,
    RendererCommandType_SetDesignResolution,
};

struct renderer_command_clear_color {
    c4f Colors;
};

struct renderer_command_set_basis {
    m44f Basis;
};

struct renderer_command_draw_textured_quad {
    renderer_texture_handle TextureHandle;
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
        Ret.Min.x = 0;
        Ret.Max.x = WindowWidth;
        
        f32 EmptyHeight = (f32)WindowHeight - OptimalWindowHeight;
        
        Ret.Min.y = (u32)(EmptyHeight * 0.5f);
        Ret.Max.y = Ret.Min.y + (u32)OptimalWindowHeight;
    }
    else {
        // NOTE(Momo): Height has priority - left and right bars
        Ret.Min.y = 0;
        Ret.Max.y = WindowHeight;
        
        
        f32 EmptyWidth = (f32)WindowWidth - OptimalWindowWidth;
        
        Ret.Min.x = (u32)(EmptyWidth * 0.5f);
        Ret.Max.x = Ret.Min.x + (u32)OptimalWindowWidth;
    }
    
    return Ret;
}



static inline void
Renderer_SetBasis(Mailbox* commands, m44f basis) {
    auto* Data = 
        commands->push_struct<renderer_command_set_basis>(RendererCommandType_SetBasis);
    Data->Basis = basis;
}

static inline void
Renderer_SetOrthoCamera(Mailbox* commands, 
                        v3f position,
                        aabb3f frustum)   
{
    auto* Data = commands->push_struct<renderer_command_set_basis>(RendererCommandType_SetBasis);
    
    auto P  = M44f_Orthographic(-1.f, 1.f,
                                -1.f, 1.f,
                                -1.f, 1.f,
                                frustum.Min.x,  
                                frustum.Max.x, 
                                frustum.Min.y, 
                                frustum.Max.y,
                                frustum.Min.z, 
                                frustum.Max.z,
                                true);
    
    m44f V = M44f_Translation(-position.x, -position.y, -position.z);
    Data->Basis = M44f_Concat(P,V);
}

static inline void
Renderer_ClearColor(Mailbox* commands, c4f colors) {
    auto* Data = commands->push_struct<renderer_command_clear_color>(RendererCommandType_ClearColor);
    Data->Colors = colors;
}



static inline void
Renderer_DrawTexturedQuad(Mailbox* commands, 
                          c4f colors, 
                          m44f transform, 
                          renderer_texture_handle texture_handle,
                          quad2f texture_coords = Quad2f_CreateDefaultUV())  

{
    auto* Data = commands->push_struct<renderer_command_draw_textured_quad>(RendererCommandType_DrawTexturedQuad);
    
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
    auto* Data = commands->push_struct<renderer_command_draw_quad>(RendererCommandType_DrawQuad);
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
    if (Line.Min.y > Line.Max.y) {
        Swap(v2f, Line.Min, Line.Max);
    }
    
    v2f LineVector = sub(Line.Max, Line.Min);
    f32 LineLength = length(LineVector);
    v2f LineMiddle = midpoint(Line.Max, Line.Min);
    
    v2f XAxis = v2f_create(1.f, 0.f);
    f32 Angle = angle_between(LineVector, XAxis);
    
    //TODO: Change line3f
    m44f T = M44f_Translation(LineMiddle.x, LineMiddle.y, PosZ);
    m44f R = M44f_RotationZ(Angle);
    m44f S = M44f_Scale(LineLength, Thickness, 1.f) ;
    
    m44f RS = M44f_Concat(R,S);
    m44f TRS = M44f_Concat(T, RS);
    
    Renderer_DrawQuad(commands, Colors, TRS);
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
    Assert(LineCount >= 3);
    f32 AngleIncrement = TAU / LineCount;
    v2f Pt1 = v2f_create(0.f, Circle.Radius); 
    v2f Pt2 = rotate(Pt1, AngleIncrement);
    
    for (u32 I = 0; I < LineCount; ++I) {
        v2f LinePt1 = Pt1 + Circle.Origin;
        v2f LinePt2 = Pt2 + Circle.Origin;
        line2f Line = Line2f_CreateFromV2f(LinePt1, LinePt2);
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
                        Line2f_Create(Aabb.Min.x, 
                                      Aabb.Min.y,  
                                      Aabb.Max.x, 
                                      Aabb.Min.y),
                        Thickness, 
                        Colors,
                        PosZ);
    // Left
    Renderer_DrawLine2f(commands, 
                        Line2f_Create(Aabb.Min.x,
                                      Aabb.Min.y,
                                      Aabb.Min.x,
                                      Aabb.Max.y),  
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Top
    Renderer_DrawLine2f(commands, 
                        Line2f_Create(Aabb.Min.x,
                                      Aabb.Max.y,
                                      Aabb.Max.x,
                                      Aabb.Max.y), 
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Right 
    Renderer_DrawLine2f(commands, 
                        Line2f_Create(Aabb.Max.x,
                                      Aabb.Min.y,
                                      Aabb.Max.x,
                                      Aabb.Max.y),  
                        Thickness, 
                        Colors,
                        PosZ);
}

static inline void 
Renderer_SetDesignResolution(Mailbox* Commands, 
                             u32 Width, 
                             u32 Height)  
{
    auto* Data = Commands->push_struct<renderer_command_set_design_resolution>(RendererCommandType_SetDesignResolution);
    Data->Width = Width;
    Data->Height = Height;
}

#endif //GAME_RENDERER_H
