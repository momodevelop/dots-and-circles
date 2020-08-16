static inline rect<2, u32> 
GetRenderRegion(u32 WindowWidth, 
                u32 WindowHeight, 
                u32 RenderWidth, 
                u32 RenderHeight) {
    Assert(RenderWidth > 0 && RenderHeight > 0 && WindowWidth > 0 && WindowHeight > 0);
    rect<2, u32> Ret = {};
    
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

static inline quad2f
UVRect2ToQuad2(rect2f Rect) {
    return {
        Rect.Min.X, Rect.Min.Y, // bottom left  	
        Rect.Max.X, Rect.Min.Y, // bottom right
        Rect.Max.X, Rect.Max.Y, // top right
        Rect.Min.X, Rect.Max.Y, // top left
    };
}

static inline void
PushCommandSetBasis(commands* Commands, m44f Basis) {
    using data_t = render_command_data_set_basis;
    auto* Data = Push<data_t>(Commands);
    Data->Basis = Basis;
}

static inline void
PushCommandSetOrthoBasis(commands* Commands, 
                         v3f Origin,
                         v3f Dimensions)   
{
    using data_t = render_command_data_set_basis;
    auto* Data = Push<data_t>(Commands);
    
    auto P  = OrthographicMatrix(-1.f, 1.f,
                                 -1.f, 1.f,
                                 -1.f, 1.f,
                                 -Dimensions.W * 0.5f,  
                                 Dimensions.W * 0.5f, 
                                 -Dimensions.H * 0.5f, 
                                 Dimensions.H* 0.5f,
                                 -Dimensions.D * 0.5f, 
                                 Dimensions.D * 0.5f,
                                 true);
    
    m44f V = TranslationMatrix(-Origin.X, -Origin.Y, 0.f);
    Data->Basis = P*V;
}

static inline void
PushCommandClearColor(commands* Commands, c4f Colors) {
    using data_t = render_command_data_clear_color;
    auto* Data = Push<data_t>(Commands);
    Data->Colors = Colors;
}

static inline void
PushCommandDrawTexturedQuad(commands* Commands, 
                            c4f Colors, 
                            m44f Transform, 
                            u32 TextureHandle,
                            quad2f TextureCoords = StandardQuadUV) 
{
    using data_t = render_command_data_draw_textured_quad;
    auto* Data = Push<data_t>(Commands);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = TextureCoords;
}


static inline void
PushCommandDrawQuad(commands* Commands, 
                    c4f Colors, 
                    m44f Transform) 
{
    using data_t = render_command_data_draw_quad;
    auto* Data = Push<data_t>(Commands);
    Data->Colors = Colors;
    Data->Transform = Transform;
}


static inline void 
PushCommandLinkTexture(commands* Commands, 
                       bitmap TextureBitmap, 
                       u32 TextureHandle) {
    using data_t = render_command_data_link_texture;
    auto* Data = Push<data_t>(Commands);
    Data->TextureBitmap = TextureBitmap;
    Data->TextureHandle = TextureHandle;
}

static inline void 
PushCommandDrawLine(commands* Payload, 
                    line2f Line, 
                    f32 Thickness = 2.f,
                    c4f Colors = {0.f, 1.f, 0.f, 1.f}) 
{
    // NOTE(Momo): Min.Y needs to be lower than Max.Y
    if (Line.Min.Y > Line.Max.Y) {
        Swap(Line.Min, Line.Max);
    }
    
    f32 LineLength = Length(Line.Max - Line.Min);
    v2f LineMiddle = Midpoint(Line.Max, Line.Min);
    
    v2f LineVector = Line.Max - Line.Min;
    f32 Angle = AngleBetween(LineVector, { 1.f, 0.f });
    
    
    m44f T = TranslationMatrix(LineMiddle.X, LineMiddle.Y, 100.f);
    m44f R = RotationZMatrix(Angle);
    m44f S = ScaleMatrix(LineLength, Thickness, 1.f) ;
    
    m44f Transform = T*R*S;
    
    PushCommandDrawQuad(Payload, Colors, Transform);
}

static inline void 
PushCommandDrawLineRect(commands* Commands, 
                        rect2f Rect,
                        f32 Thickness = 1.f,
                        c4f Colors = {0.f, 1.f, 0.f, 1.f}) 
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
PushCommandSetDesignResolution(commands* Commands, u32 Width, u32 Height)  
{
    using data_t = render_command_data_set_design_resolution;
    auto* Data = Push<data_t>(Commands);
    Data->Width = Width;
    Data->Height = Height;
}