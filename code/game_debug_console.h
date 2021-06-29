#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#define DebugConsole_LineLength 110
#define DebugConsole_InfoLineCount 5
#define DebugConsole_MaxCommands 16
#define DebugConsole_PosZ 90.f
#define DebugConsole_InfoBgColor C4F_GREY3
#define DebugConsole_InputBgColor C4F_GREY2
#define DebugConsole_InputTextColor C4F_WHITE
#define DebugConsole_Width Game_DesignWidth 
#define DebugConsole_Height 240.f
#define DebugConsole_StartPosX 0.f
#define DebugConsole_StartPosY -Game_DesignHeight/2 - DebugConsole_Height/2
#define DebugConsole_EndPosX 0.f
#define DebugConsole_EndPosY -Game_DesignHeight/2 + DebugConsole_Height/2
#define DebugConsole_TransitionDuration 0.25f
#define DebugConsole_StartPopDuration 0.5f
#define DebugConsole_PopRepeatDuration 0.1f

typedef void (*debug_console_callback)(struct debug_console* Console, void* Context, String Args);

struct debug_console_command {
    String Key;
    debug_console_callback Callback;
    void* Context;
};

struct debug_console_line {
    String_Buffer Text;
    c4f Color;
};

struct debug_console {
    b8 IsActive;
    
    v2f Position;
    
    Array<debug_console_line> InfoLines;
    debug_console_line InputLine;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    Timer StartPopRepeatTimer;
    Timer PopRepeatTimer;
    b8 IsStartPop;
    
    // Enter and Exit transitions for swag!
    Timer TransitTimer;
    
    // List of commands
    List<debug_console_command> Commands;
};


static inline b8
DebugConsole_Init(debug_console* C,
                  Arena* arena)
{
    C->TransitTimer = Timer::create(DebugConsole_TransitionDuration);
    
    C->Position = { DebugConsole_StartPosX, DebugConsole_StartPosY };
    C->StartPopRepeatTimer = Timer::create(DebugConsole_StartPopDuration);
    C->PopRepeatTimer = Timer::create(DebugConsole_PopRepeatDuration); 
    
    if (!C->Commands.alloc(arena, DebugConsole_MaxCommands)) {
        return false;
    }
    
    if (!C->InputLine.Text.alloc(arena, DebugConsole_LineLength)) {
        return false;
    }
    
    if (!C->InfoLines.alloc(arena, DebugConsole_InfoLineCount)) {
        return false;
    }
    for (u32 I = 0; I < C->InfoLines.count; ++I) {
        if (!C->InfoLines[I].Text.alloc(arena, DebugConsole_LineLength)){
            return false;
        }
    }
    
    return true;
}

static inline b8 
DebugConsole_AddCmd(debug_console* C, 
                    String Key, 
                    debug_console_callback Callback, 
                    void* Context)
{
    debug_console_command* Command = C->Commands.push();
    if (Command == nullptr) {
        return false;
    }
    Command->Key = Key;
    Command->Callback = Callback;
    Command->Context = Context;
    
    return true;
}



static inline void
DebugConsole_PushInfo(debug_console* Console, String str, c4f Color) {
    for (u32 I = 0; I < Console->InfoLines.count - 1; ++I) {
        u32 J = Console->InfoLines.count - 1 - I;
        debug_console_line* Dest = Console->InfoLines + J;
        debug_console_line* Src = Console->InfoLines + J - 1;
        Dest->Text.copy(Src->Text.str);
        Dest->Color = Src->Color;
    }
    Console->InfoLines[0].Color = Color;
    Console->InfoLines[0].Text.clear();
    Console->InfoLines[0].Text.copy(str);
}


static inline void 
DebugConsole_Pop(debug_console* Console) {
    Console->InputLine.Text.pop();
}

static inline void 
DebugConsole_RemoveCmd(debug_console* C, String Key) {
    for (u32 I = 0; I < C->Commands.count; ++I) {
        if (C->Commands[I].Key == Key) {
            C->Commands.slear(I);
            return;
        }
    }
    
}

// Returns true if there is a new command
static inline void 
DebugConsole_Update(debug_console* Console, 
                    f32 DeltaTime) 
{
    if (Button_IsPoked(G_Input->ButtonConsole)) {
        Console->IsActive = !Console->IsActive; 
    }
    
    // Transition
    {
        v2f StartPos = { DebugConsole_StartPosX, DebugConsole_StartPosY };
        v2f EndPos =  { DebugConsole_EndPosX, DebugConsole_EndPosY };
        
        f32 P = ease_in_quad(Console->TransitTimer.percent());
        v2f Delta = EndPos - StartPos; 
        
        v2f DeltaP = Delta * P;
        Console->Position = StartPos + DeltaP; 
    }
    
    if (Console->IsActive) {
        Console->TransitTimer.tick(DeltaTime);
        if (G_Input->Characters.count > 0 && 
            G_Input->Characters.count <= Console->InputLine.Text.remaining()) 
        {  
            Console->InputLine.Text.push(G_Input->Characters.str);
        }
        
        // Remove character backspace logic
        if (Button_IsDown(G_Input->ButtonBack)) {
            if(!Console->IsStartPop) {
                DebugConsole_Pop(Console);
                Console->IsStartPop = true;
                Console->StartPopRepeatTimer.reset();
                Console->PopRepeatTimer.reset();
            }
            else {
                if (Console->StartPopRepeatTimer.is_end()) {
                    if(Console->PopRepeatTimer.is_end()) {
                        DebugConsole_Pop(Console);
                        Console->PopRepeatTimer.reset();
                    }
                    Console->PopRepeatTimer.tick(DeltaTime);
                }
                Console->StartPopRepeatTimer.tick(DeltaTime);
            }
        }
        else {
            Console->IsStartPop = false; 
        }
        
        // Execute command
        
        String InputLineCStr = Console->InputLine.Text.str;
        if (Button_IsPoked(G_Input->ButtonConfirm)) {
            DebugConsole_PushInfo(Console, InputLineCStr, C4F_WHITE);
            
            u32 Min = 0;
            u32 Max = 0;
            for (u32 I = 0; I < Console->InputLine.Text.count; ++I) {
                if (Console->InputLine.Text.data[I] == ' ') {
                    Max = I;
                    break;
                }
            }
            String CommandStr = {};
            CommandStr.init(InputLineCStr, 
                            Min, 
                            Max); 
            
            // Send a command to a callback
            for (u32 I = 0; I < Console->Commands.count; ++I) {
                debug_console_command* Command = Console->Commands + I;
                if (Command->Key == CommandStr) {
                    Command->Callback(Console, 
                                      Command->Context, 
                                      InputLineCStr);
                }
            }
            Console->InputLine.Text.clear();
            
        }
    }
    else if (!Console->IsActive) {
        Console->TransitTimer.untick(DeltaTime);
    }
    
    
}


static inline void
DebugConsole_Render(debug_console* Console) 
{
    if (Console->TransitTimer.is_begin()) {
        return;
    }
    font* Font = G_Assets->Fonts + Font_Default;
    v2f Dimensions = { DebugConsole_Width, DebugConsole_Height };
    f32 Bottom = Console->Position.y - Dimensions.h * 0.5f;
    f32 Left = Console->Position.x - Dimensions.w * 0.5f;
    f32 LineHeight = Dimensions.h / (Console->InfoLines.count + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = Font_GetHeight(Font) * FontSize;
    
    f32 PaddingHeight =
        (LineHeight - FontHeight) * 0.5f  + ABS(Font->Descent) * FontSize; 
    f32 PaddingWidth = Dimensions.w * 0.005f;
    {
        m44f S = m44f::create_scale(Dimensions.x, 
                                    Dimensions.y, 
                                    1.f);
        
        m44f P = m44f::create_translation(Console->Position.x,
                                          Console->Position.y,
                                          DebugConsole_PosZ);
        Renderer_DrawQuad(G_Renderer, 
                          DebugConsole_InfoBgColor, 
                          P * S);
    }
    
    {
        m44f S = m44f::create_scale(Dimensions.w, LineHeight, 0.f);
        m44f P = m44f::create_translation(Console->Position.x, 
                                          Bottom + LineHeight * 0.5f,
                                          DebugConsole_PosZ+ 0.01f);
        
        
        Renderer_DrawQuad(G_Renderer, 
                          DebugConsole_InputBgColor, 
                          P * S);
    }
    
    // Draw info text
    {
        for (u32 I = 0; I < Console->InfoLines.count ; ++I) {
            v3f Position = {};
            Position.x = Left + PaddingWidth;
            Position.y = Bottom + ((I+1) * LineHeight) + PaddingHeight;
            Position.z = DebugConsole_PosZ + 0.01f;
            
            String InfoLineCStr = Console->InfoLines[I].Text.str;
            Draw_Text(Font_Default, 
                      Position,
                      InfoLineCStr,
                      FontSize,
                      Console->InfoLines[I].Color);
        }
        
        v3f Position = {};
        Position.x = Left + PaddingWidth;
        Position.y = Bottom + PaddingHeight;
        Position.z = DebugConsole_PosZ + 0.02f;
        
        String InputLineCStr = Console->InputLine.Text.str;
        Draw_Text(Font_Default, 
                  Position,
                  InputLineCStr,
                  FontSize,
                  DebugConsole_InputTextColor);
        
        
    }
}





#endif
