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
                    f32 Thickness = 1.f,
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
PushCommandDrawLineRect(commands* Payload, 
                        rect2f Rect,
                        f32 Thickness = 1.f,
                        c4f Colors = {0.f, 1.f, 0.f, 1.f}) 
{
    //Bottom
    PushCommandDrawLine(Payload, 
                        { 
                            Rect.Min.X, 
                            Rect.Min.Y,  
                            Rect.Max.X, 
                            Rect.Min.Y,
                        },  Thickness, Colors);
    // Left
    PushCommandDrawLine(Payload, 
                        { 
                            Rect.Min.X,
                            Rect.Min.Y,
                            Rect.Min.X,
                            Rect.Max.Y,
                        },  Thickness, Colors);
    
    //Top
    PushCommandDrawLine(Payload, 
                        { 
                            Rect.Min.X,
                            Rect.Max.Y,
                            Rect.Max.X,
                            Rect.Max.Y,
                        }, Thickness, Colors);
    
    //Right 
    PushCommandDrawLine(Payload, 
                        { 
                            Rect.Max.X,
                            Rect.Min.Y,
                            Rect.Max.X,
                            Rect.Max.Y,
                        },  Thickness, Colors);
}

