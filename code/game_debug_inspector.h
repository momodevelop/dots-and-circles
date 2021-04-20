/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H

#define DebugInspector_PosX -Game_DesignWidth * 0.5f + 10.f
#define DebugInspector_PosY Game_DesignHeight * 0.5f - 32.f
#define DebugInspector_PosZ 91.f
#define DebugInspector_EntryTextSize 256
#define DebugInspector_EntryCap 32

struct debug_inspector_entry {
    u8_str Text;
};


struct debug_inspector {
    b32 IsActive;
    
    // TODO(Momo): Create list
    debug_inspector_entry Entries[DebugInspector_EntryCap];
    u32 EntryCount;
};

static inline void
DebugInspector_Init(debug_inspector* Inspector, arena* Arena) {
    Inspector->EntryCount = 0;
    Inspector->IsActive = False;
    
    for (u32 I = 0; I < ArrayCount(Inspector->Entries); ++I) {
        debug_inspector_entry* Entry = Inspector->Entries + I;
        Entry->Text = U8Str_CreateFromArena(Arena, DebugInspector_EntryTextSize);
        
    }
}

static inline void
DebugInspector_Begin(debug_inspector* Inspector) {
    if(!Inspector->IsActive)
        return;
    
    Inspector->EntryCount = 0;
}

static inline void 
DebugInspector_End(debug_inspector* Inspector, 
                   mailbox* RenderCommands,
                   assets* Assets) {
    if(!Inspector->IsActive)
        return;
    
    // TODO(Momo): Complete this
    // For each variable, render:
    // Name: Data
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < Inspector->EntryCount; ++I) {
        debug_inspector_entry* Entry = Inspector->Entries + I;
        v3f Position = V3f_Create(DebugInspector_PosX, 
                                  DebugInspector_PosY + OffsetY,
                                  DebugInspector_PosZ); 
        Draw_Text(RenderCommands, 
                  Assets, 
                  Font_Default, 
                  Position, 
                  Entry->Text.CStr,
                  32.f, 
                  Color_White);
        U8Str_Clear(&Entry->Text);
        OffsetY -= 32.f;
    }
}

static inline debug_inspector_entry*
DebugInspector_PushEntry(debug_inspector* Inspector, u8_cstr Label) {
    Assert(Inspector->EntryCount < ArrayCount(Inspector->Entries));
    debug_inspector_entry* Entry = Inspector->Entries + Inspector->EntryCount++;
    U8Str_Copy(&Entry->Text, Label);
    return Entry;
}

static inline void
DebugInspector_PushU32(debug_inspector* Inspector, u8_cstr Label, u32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    debug_inspector_entry* Entry = DebugInspector_PushEntry(Inspector, Label);
    U8Str_PushU32(&Entry->Text, Item);
}

static inline void
DebugInspector_PushS32(debug_inspector* Inspector, u8_cstr Label, i32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    debug_inspector_entry* Entry = DebugInspector_PushEntry(Inspector, Label);
    U8Str_PushS32(&Entry->Text, Item);
}

#endif //GAME_DEBUG_INSPECTOR_H
