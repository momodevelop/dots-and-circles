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
Renderer_SetBasis(mailbox* Commands, m44f Basis) {
    auto* Data = Mailbox_PushStruct(renderer_command_set_basis,
                                    Commands, 
                                    RendererCommandType_SetBasis);
    Data->Basis = Basis;
}

static inline void
Renderer_SetOrthoCamera(mailbox* Commands, 
                        v3f Position,
                        aabb3f Frustum)   
{
    auto* Data = Mailbox_PushStruct(renderer_command_set_basis,
                                    Commands, 
                                    RendererCommandType_SetBasis);
    
    auto P  = M44f_Orthographic(-1.f, 1.f,
                                -1.f, 1.f,
                                -1.f, 1.f,
                                Frustum.Min.X,  
                                Frustum.Max.X, 
                                Frustum.Min.Y, 
                                Frustum.Max.Y,
                                Frustum.Min.Z, 
                                Frustum.Max.Z,
                                true);
    
    m44f V = M44f_Translation(-Position.X, -Position.Y, -Position.Z);
    Data->Basis = M44f_Concat(P,V);
}

static inline void
Renderer_ClearColor(mailbox* Commands, c4f Colors) {
    auto* Data = Mailbox_PushStruct(renderer_command_clear_color,
                                    Commands,
                                    RendererCommandType_ClearColor);
    Data->Colors = Colors;
}



static inline void
Renderer_DrawTexturedQuad(mailbox* Commands, 
                          c4f Colors, 
                          m44f Transform, 
                          renderer_texture_handle TextureHandle,
                          quad2f TextureCoords = Quad2f_CreateDefaultUV())  
{
    using data_t = renderer_command_draw_textured_quad;
    auto* Data = Mailbox_PushStruct(renderer_command_draw_textured_quad,
                                    Commands, 
                                    RendererCommandType_DrawTexturedQuad);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = TextureCoords;
}

static inline void
Renderer_DrawQuad(mailbox* Commands, 
                  c4f Colors, 
                  m44f Transform) 
{
    auto* Data = Mailbox_PushStruct(renderer_command_draw_quad,
                                    Commands, 
                                    RendererCommandType_DrawQuad);
    Data->Colors = Colors;
    Data->Transform = Transform;
}

static inline void 
Renderer_DrawLine2f(mailbox* Payload, 
                    line2f Line,
                    f32 Thickness,
                    c4f Colors,
                    f32 PosZ) 
{
    // NOTE(Momo): Min.Y needs to be lower than Max.Y
    if (Line.Min.Y > Line.Max.Y) {
        Swap(v2f, Line.Min, Line.Max);
    }
    
    v2f LineVector = V2f_Sub(Line.Max, Line.Min);
    f32 LineLength = V2f_Length(LineVector);
    v2f LineMiddle = V2f_Midpoint(Line.Max, Line.Min);
    
    v2f XAxis = V2f_Create(1.f, 0.f);
    f32 Angle = V2f_AngleBetween(LineVector, XAxis);
    
    //TODO: Change line3f
    m44f T = M44f_Translation(LineMiddle.X, LineMiddle.Y, PosZ);
    m44f R = M44f_RotationZ(Angle);
    m44f S = M44f_Scale(LineLength, Thickness, 1.f) ;
    
    m44f RS = M44f_Concat(R,S);
    m44f TRS = M44f_Concat(T, RS);
    
    Renderer_DrawQuad(Payload, Colors, TRS);
}

static inline void
Renderer_DrawCircle2f(mailbox* Commands,
                      circle2f Circle,
                      f32 Thickness, 
                      u32 LineCount,
                      c4f Color,
                      f32 PosZ) 
{
    // NOTE(Momo): Essentially a bunch of lines
    // We can't really have a surface with less than 3 lines
    Assert(LineCount >= 3);
    f32 AngleIncrement = Tau32 / LineCount;
    v2f Pt1 = V2f_Create(0.f, Circle.Radius); 
    v2f Pt2 = V2f_Rotate(Pt1, AngleIncrement);
    
    for (u32 I = 0; I < LineCount; ++I) {
        v2f LinePt1 = V2f_Add(Pt1, Circle.Origin);
        v2f LinePt2 = V2f_Add(Pt2, Circle.Origin);
        line2f Line = Line2f_CreateFromV2f(LinePt1, LinePt2);
        Renderer_DrawLine2f(Commands, 
                            Line,
                            Thickness,
                            Color,
                            PosZ);
        
        Pt1 = Pt2;
        Pt2 = V2f_Rotate(Pt1, AngleIncrement);
        
    }
}

static inline void 
Renderer_DrawAabb2f(mailbox* Commands, 
                    aabb2f Aabb,
                    f32 Thickness,
                    c4f Colors,
                    f32 PosZ ) 
{
    //Bottom
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.Min.X, 
                                      Aabb.Min.Y,  
                                      Aabb.Max.X, 
                                      Aabb.Min.Y),
                        Thickness, 
                        Colors,
                        PosZ);
    // Left
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.Min.X,
                                      Aabb.Min.Y,
                                      Aabb.Min.X,
                                      Aabb.Max.Y),  
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Top
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.Min.X,
                                      Aabb.Max.Y,
                                      Aabb.Max.X,
                                      Aabb.Max.Y), 
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Right 
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.Max.X,
                                      Aabb.Min.Y,
                                      Aabb.Max.X,
                                      Aabb.Max.Y),  
                        Thickness, 
                        Colors,
                        PosZ);
}

static inline void 
Renderer_SetDesignResolution(mailbox* Commands, 
                             u32 Width, 
                             u32 Height)  
{
    auto* Data = Mailbox_PushStruct(renderer_command_set_design_resolution,
                                    Commands, 
                                    RendererCommandType_SetDesignResolution);
    Data->Width = Width;
    Data->Height = Height;
}

#endif //GAME_RENDERER_H
