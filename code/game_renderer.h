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
    b32 Success;
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
    MM_M44f Basis;
};

struct renderer_command_draw_textured_quad {
    renderer_texture_handle TextureHandle;
    c4f Colors;
    MM_M44f Transform;
    MM_Quad2f TextureCoords; 
};


struct renderer_command_draw_quad {
    c4f Colors;
    MM_M44f Transform;
};

struct renderer_command_set_design_resolution {
    u32 Width;
    u32 Height;
};

static inline MM_Aabb2u 
Renderer_CalcRenderRegion(u32 WindowWidth, 
                          u32 WindowHeight, 
                          u32 RenderWidth, 
                          u32 RenderHeight) 
{
    if ( RenderWidth == 0 || RenderHeight == 0 || WindowWidth == 0 || WindowHeight == 0) {
        return {};
    }
    MM_Aabb2u Ret = {};
    
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
Renderer_SetBasis(mailbox* Commands, MM_M44f Basis) {
    auto* Data = Mailbox_PushStruct(renderer_command_set_basis,
                                    Commands, 
                                    RendererCommandType_SetBasis);
    Data->Basis = Basis;
}

static inline void
Renderer_SetOrthoCamera(mailbox* Commands, 
                        MM_V3f Position,
                        MM_Aabb3f Frustum)   
{
    auto* Data = Mailbox_PushStruct(renderer_command_set_basis,
                                    Commands, 
                                    RendererCommandType_SetBasis);
    
    auto P  = MM_M44f_Orthographic(-1.f, 1.f,
                                -1.f, 1.f,
                                -1.f, 1.f,
                                Frustum.min.x,  
                                Frustum.max.x, 
                                Frustum.min.y, 
                                Frustum.max.y,
                                Frustum.min.z, 
                                Frustum.max.z,
                                true);
    
    MM_M44f V = MM_M44f_Translation(-Position.x, -Position.y, -Position.z);
    Data->Basis = MM_M44f_Concat(P,V);
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
                          MM_M44f Transform, 
                          renderer_texture_handle TextureHandle,
                          MM_Quad2f TextureCoords = MM_Quad2f_CreateDefaultUV())  
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
                  MM_M44f Transform) 
{
    auto* Data = Mailbox_PushStruct(renderer_command_draw_quad,
                                    Commands, 
                                    RendererCommandType_DrawQuad);
    Data->Colors = Colors;
    Data->Transform = Transform;
}

static inline void 
Renderer_DrawLine2f(mailbox* Payload, 
                    MM_Line2f Line,
                    f32 Thickness,
                    c4f Colors,
                    f32 PosZ) 
{
    // NOTE(Momo): Min.y needs to be lower than Max.y
    if (Line.min.y > Line.max.y) {
        Swap(MM_V2f, Line.min, Line.max);
    }
    
    MM_V2f LineVector = MM_V2f_Sub(Line.max, Line.min);
    f32 LineLength = MM_V2f_Length(LineVector);
    MM_V2f LineMiddle = MM_V2f_Midpoint(Line.max, Line.min);
    
    MM_V2f XAxis = MM_V2f_Create(1.f, 0.f);
    f32 Angle = MM_V2f_AngleBetween(LineVector, XAxis);
    
    //TODO: Change line3f
    MM_M44f T = MM_M44f_Translation(LineMiddle.x, LineMiddle.y, PosZ);
    MM_M44f R = MM_M44f_RotationZ(Angle);
    MM_M44f S = MM_M44f_Scale(LineLength, Thickness, 1.f) ;
    
    MM_M44f RS = MM_M44f_Concat(R,S);
    MM_M44f TRS = MM_M44f_Concat(T, RS);
    
    Renderer_DrawQuad(Payload, Colors, TRS);
}

static inline void
Renderer_DrawCircle2f(mailbox* Commands,
                      MM_Circle2f Circle,
                      f32 Thickness, 
                      u32 LineCount,
                      c4f Color,
                      f32 PosZ) 
{
    // NOTE(Momo): Essentially a bunch of lines
    // We can't really have a surface with less than 3 lines
    Assert(LineCount >= 3);
    f32 AngleIncrement = MM_Math_Tau32 / LineCount;
    MM_V2f Pt1 = MM_V2f_Create(0.f, Circle.radius); 
    MM_V2f Pt2 = MM_V2f_Rotate(Pt1, AngleIncrement);
    
    for (u32 I = 0; I < LineCount; ++I) {
        MM_V2f LinePt1 = MM_V2f_Add(Pt1, Circle.origin);
        MM_V2f LinePt2 = MM_V2f_Add(Pt2, Circle.origin);
        MM_Line2f Line = Line2f_CreateFromV2f(LinePt1, LinePt2);
        Renderer_DrawLine2f(Commands, 
                            Line,
                            Thickness,
                            Color,
                            PosZ);
        
        Pt1 = Pt2;
        Pt2 = MM_V2f_Rotate(Pt1, AngleIncrement);
        
    }
}

static inline void 
Renderer_DrawAabb2f(mailbox* Commands, 
                    MM_Aabb2f Aabb,
                    f32 Thickness,
                    c4f Colors,
                    f32 PosZ ) 
{
    //Bottom
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.min.x, 
                                      Aabb.min.y,  
                                      Aabb.max.x, 
                                      Aabb.min.y),
                        Thickness, 
                        Colors,
                        PosZ);
    // Left
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.min.x,
                                      Aabb.min.y,
                                      Aabb.min.x,
                                      Aabb.max.y),  
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Top
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.min.x,
                                      Aabb.max.y,
                                      Aabb.max.x,
                                      Aabb.max.y), 
                        Thickness, 
                        Colors,
                        PosZ);
    
    //Right 
    Renderer_DrawLine2f(Commands, 
                        Line2f_Create(Aabb.max.x,
                                      Aabb.min.y,
                                      Aabb.max.x,
                                      Aabb.max.y),  
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
