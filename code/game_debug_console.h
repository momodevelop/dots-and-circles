#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__

#define DebugConsole_CharactersPerLine 110
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
    u8 Buffer[DebugConsole_CharactersPerLine];
    u8_str Text;
    c4f Color;
};

struct debug_console_command_list {
    u32 Capacity;
    u32 Count;
    debug_console_command* E;
};


struct debug_console {
    b32 IsActive;
    
    v2f Position;
    
    debug_console_line InfoLines[DebugConsole_InfoLineCount];
    debug_console_line InputLine;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    timer StartPopRepeatTimer;
    timer PopRepeatTimer;
    b32 IsStartPop;
    
    // Enter and Exit transitions for swag!
    timer TransitTimer;
    
    // List of commands
    debug_console_command Commands[DebugConsole_MaxCommands];
    u32 CommandCount;
};

static inline void 
DebugConsole_AddCmd(debug_console* C, 
                    u8_cstr Key, 
                    debug_console_callback Callback, 
                    void* Context)
{
    debug_console_command Command = {};
    Command.Key = Key;
    Command.Callback = Callback;
    Command.Context = Context;
    
    Assert(C->CommandCount < ArrayCount(C->Commands));
    C->Commands[C->CommandCount++] = Command; 
}


static inline void 
DebugConsole_RemoveCmd(debug_console* Console, u8_cstr Key) {
    for (u32 I = 0; I < Console->CommandCount; ++I) {
        if (U8CStr_Compare(Console->Commands[I].Key, Key)) {
            // Swap with last element
            Console->Commands[I] = Console->Commands[Console->CommandCount-1];
            --Console->CommandCount;
            return;
        }
    }
    
}

static inline void
DebugConsole_Create(debug_console* Console,
                    arena* Arena)
{
    for (usize I = 0; I < ArrayCount(Console->InfoLines); ++I) {
        debug_console_line* InfoLine = Console->InfoLines + I;
        InfoLine->Text = U8Str_CreateFromBuffer(InfoLine->Buffer);
    }
    
    Console->InputLine.Text = U8Str_CreateFromBuffer(Console->InputLine.Buffer);
    Console->CommandCount = 0;
    
    Console->TransitTimer = Timer_Create(DebugConsole_TransitionDuration);
    
    Console->Position = V2f_Create(DebugConsole_StartPosX, DebugConsole_StartPosY);
    Console->StartPopRepeatTimer = Timer_Create(DebugConsole_StartPopDuration);
    Console->PopRepeatTimer = Timer_Create(DebugConsole_PopRepeatDuration); 
}


static inline void
PushInfo(debug_console* Console, u8_cstr String, c4f Color) {
    for (u32 I = 0; I < ArrayCount(Console->InfoLines) - 1; ++I) {
        u32 J = ArrayCount(Console->InfoLines) - 1 - I;
        debug_console_line* Dest = Console->InfoLines + J;
        debug_console_line* Src = Console->InfoLines + J - 1;
        U8Str_Copy(&Dest->Text, Src->Text.CStr);
        Dest->Color = Src->Color;
    }
    Console->InfoLines[0].Color = Color;
    U8Str_Clear(&Console->InfoLines[0].Text);
    U8Str_Copy(&Console->InfoLines[0].Text, String);
}

static inline void 
DebugConsole_Pop(debug_console* Console) {
    U8Str_Pop(&Console->InputLine.Text);
}

// Returns true if there is a new command
static inline void 
DebugConsole_Update(debug_console* Console, 
                    game_input* Input,
                    f32 DeltaTime) 
{
    if (IsPoked(Input->ButtonConsole)) {
        Console->IsActive = !Console->IsActive; 
    }
    
    // Transition
    {
        v2f StartPos = V2f_Create(DebugConsole_StartPosX, DebugConsole_StartPosY);
        v2f EndPos = V2f_Create(DebugConsole_EndPosX, DebugConsole_EndPosY);
        
        f32 P = EaseInQuad(Timer_Percent(Console->TransitTimer));
        v2f Delta = V2f_Sub(EndPos, StartPos); 
        
        v2f DeltaP = V2f_Mul(Delta, P);
        Console->Position = V2f_Add(StartPos, DeltaP); 
    }
    
    if (Console->IsActive) {
        Timer_Tick(&Console->TransitTimer, DeltaTime);
        if (Input->Characters.Size > 0 && 
            Input->Characters.Size <= U8Str_Remaining(&Console->InputLine.Text)) 
        {  
            U8Str_PushCStr(&Console->InputLine.Text, Input->Characters.CStr);
        }
        
        // Remove character backspace logic
        if (IsDown(Input->ButtonBack)) {
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
        if (IsPoked(Input->ButtonConfirm)) {
            PushInfo(Console, Console->InputLine.Text.CStr, Color_White);
            
            u32 Min = 0;
            u32 Max = 0;
            for (u32 I = 0; I < Console->InputLine.Text.Size; ++I) {
                if (Console->InputLine.Text.Data[I] == ' ') {
                    Max = I;
                    break;
                }
            }
            u8_cstr CommandStr = U8CStr_SubString(Console->InputLine.Text.CStr, 
                                                  Min, 
                                                  Max); 
            
            // Send a command to a callback
            for (u32 I = 0; I < Console->CommandCount; ++I) {
                debug_console_command* Command = Console->Commands + I;
                if (U8CStr_Compare(Command->Key, CommandStr)) {
                    Command->Callback(Console, 
                                      Command->Context, 
                                      Console->InputLine.Text.CStr);
                }
            }
            U8Str_Clear(&Console->InputLine.Text);
            
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
    v2f Dimensions = V2f_Create( DebugConsole_Width, DebugConsole_Height );
    f32 Bottom = Console->Position.Y - Dimensions.H * 0.5f;
    f32 Left = Console->Position.X - Dimensions.W * 0.5f;
    f32 LineHeight = Dimensions.H / (ArrayCount(Console->InfoLines) + 1);
    f32 FontSize = LineHeight * 0.9f;
    f32 FontHeight = Font_GetHeight(Font) * FontSize;
    
    f32 PaddingHeight =
        (LineHeight - FontHeight) * 0.5f  + AbsOf(Font->Descent) * FontSize; 
    f32 PaddingWidth = Dimensions.W * 0.005f;
    {
        m44f ScaleMatrix = M44f_Scale(Dimensions.X, 
                                      Dimensions.Y, 
                                      1.f);
        
        m44f PositionMatrix = M44f_Translation(Console->Position.X,
                                               Console->Position.Y,
                                               DebugConsole_PosZ);
        m44f InfoBgTransform = M44f_Concat(PositionMatrix, ScaleMatrix);
        PushDrawQuad(RenderCommands, 
                     DebugConsole_InfoBgColor, 
                     InfoBgTransform);
    }
    
    // Draw input text
    {
        m44f ScaleMatrix = M44f_Scale(Dimensions.W, LineHeight, 0.f);
        m44f PositionMatrix = M44f_Translation(Console->Position.X, 
                                               Bottom + LineHeight * 0.5f,
                                               DebugConsole_PosZ+ 0.01f);
        
        m44f InputBgTransform = M44f_Concat(PositionMatrix, ScaleMatrix);
        PushDrawQuad(RenderCommands, 
                     DebugConsole_InputBgColor, 
                     InputBgTransform);
    }
    
    // Draw info text
    {
        for (u32 I = 0; I < ArrayCount(Console->InfoLines) ; ++I) {
            v3f Position = {};
            Position.X = Left + PaddingWidth;
            Position.Y = Bottom + ((I+1) * LineHeight) + PaddingHeight;
            Position.Z = DebugConsole_PosZ + 0.01f;
            
            DrawText(RenderCommands,
                     Assets,
                     Font_Default, 
                     Position,
                     Console->InfoLines[I].Text.CStr,
                     FontSize,
                     Console->InfoLines[I].Color);
        }
        
        v3f Position = {};
        Position.X = Left + PaddingWidth;
        Position.Y = Bottom + PaddingHeight;
        Position.Z = DebugConsole_PosZ + 0.02f;
        
        DrawText(RenderCommands, 
                 Assets, 
                 Font_Default, 
                 Position,
                 Console->InputLine.Text.CStr,
                 FontSize,
                 DebugConsole_InputTextColor);
        
        
    }
}





#endif
