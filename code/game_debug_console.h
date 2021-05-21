#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#define DebugConsole_LineLength 110
#define DebugConsole_InfoLineCount 5
#define DebugConsole_MaxCommands 16
#define DebugConsole_PosZ 90.f
#define DebugConsole_InfoBgColor Color_Grey3
#define DebugConsole_InputBgColor Color_Grey2
#define DebugConsole_InputTextColor Color_White
#define DebugConsole_Width Game_DesignWidth 
#define DebugConsole_Height 240.f
#define DebugConsole_StartPosX 0.f
#define DebugConsole_StartPosY -Game_DesignHeight/2 - DebugConsole_Height/2
#define DebugConsole_EndPosX 0.f
#define DebugConsole_EndPosY -Game_DesignHeight/2 + DebugConsole_Height/2
#define DebugConsole_TransitionDuration 0.25f
#define DebugConsole_StartPopDuration 0.5f
#define DebugConsole_PopRepeatDuration 0.1f

typedef void (*debug_console_callback)(struct debug_console* Console, void* Context, u8_cstr Args);

struct debug_console_command {
    u8_cstr Key;
    debug_console_callback Callback;
    void* Context;
};

struct debug_console_line {
    u8_str Text;
    c4f Color;
};

struct debug_console {
    b32 IsActive;
    
    MM_V2f Position;
    
    MM_Array<debug_console_line> InfoLines;
    debug_console_line InputLine;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    timer StartPopRepeatTimer;
    timer PopRepeatTimer;
    b32 IsStartPop;
    
    // Enter and Exit transitions for swag!
    timer TransitTimer;
    
    // List of commands
    MM_List<debug_console_command> Commands;
};


static inline void
DebugConsole_Init(debug_console* C,
                  MM_Arena* Arena)
{
    C->TransitTimer = Timer_Create(DebugConsole_TransitionDuration);
    
    C->Position = MM_V2f_Create(DebugConsole_StartPosX, DebugConsole_StartPosY);
    C->StartPopRepeatTimer = Timer_Create(DebugConsole_StartPopDuration);
    C->PopRepeatTimer = Timer_Create(DebugConsole_PopRepeatDuration); 
    
    MM_List_New(&C->Commands, Arena, DebugConsole_MaxCommands);
    MM_U8Str_New(&C->InputLine.Text, Arena, DebugConsole_LineLength);
    
    MM_Array_New(&C->InfoLines, Arena, DebugConsole_InfoLineCount);
    for (u32 I = 0; I < C->InfoLines.count; ++I) {
        MM_U8Str_New(&C->InfoLines[I].Text, Arena, DebugConsole_LineLength);
    }
    
}

static inline void 
DebugConsole_AddCmd(debug_console* C, 
                    u8_cstr Key, 
                    debug_console_callback Callback, 
                    void* Context)
{
    debug_console_command* Command = MM_List_Push(&C->Commands);
    Command->Key = Key;
    Command->Callback = Callback;
    Command->Context = Context;
    
}



static inline void
DebugConsole_PushInfo(debug_console* Console, u8_cstr String, c4f Color) {
    for (u32 I = 0; I < Console->InfoLines.count - 1; ++I) {
        u32 J = Console->InfoLines.count - 1 - I;
        debug_console_line* Dest = Console->InfoLines + J;
        debug_console_line* Src = Console->InfoLines + J - 1;
        MM_U8Str_Copy(&Dest->Text, &Src->Text);
        Dest->Color = Src->Color;
    }
    Console->InfoLines[0].Color = Color;
    MM_U8Str_Clear(&Console->InfoLines[0].Text);
    MM_U8Str_CopyCStr(&Console->InfoLines[0].Text, String);
}


static inline void 
DebugConsole_Pop(debug_console* Console) {
    MM_U8Str_Pop(&Console->InputLine.Text);
}

static inline void 
DebugConsole_RemoveCmd(debug_console* C, u8_cstr Key) {
    for (u32 I = 0; I < C->Commands.count; ++I) {
        if (MM_U8CStr_Cmp(C->Commands.data[I].Key, Key)) {
            MM_List_Slear(&C->Commands, I);
            return;
        }
    }
    
}

// Returns true if there is a new command
static inline void 
DebugConsole_Update(debug_console* Console, 
                    platform_input* Input,
                    f32 DeltaTime) 
{
    if (Button_IsPoked(Input->ButtonConsole)) {
        Console->IsActive = !Console->IsActive; 
    }
    
    // Transition
    {
        MM_V2f StartPos = MM_V2f_Create(DebugConsole_StartPosX, DebugConsole_StartPosY);
        MM_V2f EndPos = MM_V2f_Create(DebugConsole_EndPosX, DebugConsole_EndPosY);
        
        f32 P = MM_Ease_InQuad(Timer_Percent(Console->TransitTimer));
        MM_V2f Delta = MM_V2f_Sub(EndPos, StartPos); 
        
        MM_V2f DeltaP = MM_V2f_Mul(Delta, P);
        Console->Position = MM_V2f_Add(StartPos, DeltaP); 
    }
    
    if (Console->IsActive) {
        Timer_Tick(&Console->TransitTimer, DeltaTime);
        if (Input->Characters.count > 0 && 
            Input->Characters.count <= MM_U8Str_Remaining(&Console->InputLine.Text)) 
        {  
            MM_U8Str_PushCStr(&Console->InputLine.Text, Input->Characters.cstr);
        }
        
        // Remove character backspace logic
        if (Button_IsDown(Input->ButtonBack)) {
            if(!Console->IsStartPop) {
                DebugConsole_Pop(Console);
                Console->IsStartPop = true;
                Timer_Reset(&Console->StartPopRepeatTimer);
                Timer_Reset(&Console->PopRepeatTimer);
            }
            else {
                if (Timer_IsEnd(Console->StartPopRepeatTimer)) {
                    if(Timer_IsEnd(Console->PopRepeatTimer)) {
                        DebugConsole_Pop(Console);
                        Timer_Reset(&Console->PopRepeatTimer);
                    }
                    Timer_Tick(&Console->PopRepeatTimer, DeltaTime);
                }
                Timer_Tick(&Console->StartPopRepeatTimer, DeltaTime);
            }
        }
        else {
            Console->IsStartPop = false; 
        }
        
        // Execute command
        
        u8_cstr InputLineCStr = Console->InputLine.Text.cstr;
        if (Button_IsPoked(Input->ButtonConfirm)) {
            DebugConsole_PushInfo(Console, InputLineCStr, Color_White);
            
            u32 Min = 0;
            u32 Max = 0;
            for (u32 I = 0; I < Console->InputLine.Text.count; ++I) {
                if (Console->InputLine.Text[I] == ' ') {
                    Max = I;
                    break;
                }
            }
            u8_cstr CommandStr = {};
            MM_U8CStr_SubString(&CommandStr,
                                InputLineCStr, 
                                Min, 
                                Max); 
            
            // Send a command to a callback
            for (u32 I = 0; I < Console->Commands.count; ++I) {
                debug_console_command* Command = Console->Commands + I;
                if (MM_U8CStr_Cmp(Command->Key, CommandStr)) {
                    Command->Callback(Console, 
                                      Command->Context, 
                                      InputLineCStr);
                }
            }
            MM_U8Str_Clear(&Console->InputLine.Text);
            
        }
    }
    else if (!Console->IsActive) {
        Timer_Untick(&Console->TransitTimer, DeltaTime);
    }
    
    
}


static inline void
DebugConsole_Render(debug_console* Console, 
                    mailbox* RenderCommands,
                    assets* Assets) 
{
    if (Timer_IsBegin(Console->TransitTimer)) {
        return;
    }
    font* Font = Assets->Fonts + Font_Default;
    MM_V2f Dimensions = MM_V2f_Create( DebugConsole_Width, DebugConsole_Height );
    f32 Bottom = Console->Position.y - Dimensions.H * 0.5f;
    f32 Left = Console->Position.x - Dimensions.W * 0.5f;
    f32 LineHeight = Dimensions.H / (Console->InfoLines.count + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = Font_GetHeight(Font) * FontSize;
    
    f32 PaddingHeight =
        (LineHeight - FontHeight) * 0.5f  + AbsOf(Font->Descent) * FontSize; 
    f32 PaddingWidth = Dimensions.W * 0.005f;
    {
        MM_M44f ScaleMatrix = MM_M44f_Scale(Dimensions.x, 
                                      Dimensions.y, 
                                      1.f);
        
        MM_M44f PositionMatrix = MM_M44f_Translation(Console->Position.x,
                                               Console->Position.y,
                                               DebugConsole_PosZ);
        MM_M44f InfoBgTransform = MM_M44f_Concat(PositionMatrix, ScaleMatrix);
        Renderer_DrawQuad(RenderCommands, 
                          DebugConsole_InfoBgColor, 
                          InfoBgTransform);
    }
    
    {
        MM_M44f ScaleMatrix = MM_M44f_Scale(Dimensions.W, LineHeight, 0.f);
        MM_M44f PositionMatrix = MM_M44f_Translation(Console->Position.x, 
                                               Bottom + LineHeight * 0.5f,
                                               DebugConsole_PosZ+ 0.01f);
        
        MM_M44f InputBgTransform = MM_M44f_Concat(PositionMatrix, ScaleMatrix);
        Renderer_DrawQuad(RenderCommands, 
                          DebugConsole_InputBgColor, 
                          InputBgTransform);
    }
    
    // Draw info text
    {
        for (u32 I = 0; I < Console->InfoLines.count ; ++I) {
            MM_V3f Position = {};
            Position.x = Left + PaddingWidth;
            Position.y = Bottom + ((I+1) * LineHeight) + PaddingHeight;
            Position.z = DebugConsole_PosZ + 0.01f;
            
            u8_cstr InfoLineCStr = Console->InfoLines[I].Text.cstr;
            Draw_Text(RenderCommands,
                      Assets,
                      Font_Default, 
                      Position,
                      InfoLineCStr,
                      FontSize,
                      Console->InfoLines[I].Color);
        }
        
        MM_V3f Position = {};
        Position.x = Left + PaddingWidth;
        Position.y = Bottom + PaddingHeight;
        Position.z = DebugConsole_PosZ + 0.02f;
        
        u8_cstr InputLineCStr = Console->InputLine.Text.cstr;
        Draw_Text(RenderCommands, 
                  Assets, 
                  Font_Default, 
                  Position,
                  InputLineCStr,
                  FontSize,
                  DebugConsole_InputTextColor);
        
        
    }
}





#endif
