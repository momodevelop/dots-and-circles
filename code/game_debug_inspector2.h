/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H

struct debug_inspector_entry {
    u8 Buffer[256];
    u8_str Text;
};


struct debug_inspector {
    b32 IsActive;
    debug_inspector_entry Entries[32];
    u32 EntryCount;
};

static inline void
DebugInspector_Init(debug_inspector* Inspector) {
    Inspector->VarCount = 0;
    Inspector->IsActive = False;
    
    for (u32 I = 0; I < ArrayCount(Inspector->Entries); ++I) {
        debug_inspector_entry* Entry = Inspector->Entries + I;
        Entry->String = U8Str_CreateFromBuffer(Entry->Buffer);
    }
}

static inline void
DebugInspector_Begin(debug_inspector* Inspector) {
    Inspector->EntryCount = 0;
}

static inline void 
DebugInspector_End(debug_inspector* Inspector, mailbox* RenderCommands) {
    // TODO(Momo): Complete this
    // For each variable, render:
    // Name: Data
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < Inspector->EntryCount; ++I) {
        debug_inspector_entry* Entry = Inspector->Entries + I;
        DrawText(RenderCommands, 
                 Assets, 
                 Font_Default, 
                 v3f{ -800.f + 10.f, 450.f - 32.f + OffsetY, 0.f }, 
                 Entry->Text.Str,
                 32.f, 
                 Color_White);
        U8Str_Clear(&Buffer);
        OffsetY -= 32.f;
    }
}

static inline void
DebugInspector_PushEntry(debug_inspector* Inspector, u8_cstr Label) {
    Assert(Inspector->EntryCount < ArrayCount(Inspector->Entries));
    debug_inspector_entry* Entry = Inspector->Entries + Inspector->EntryCount++;
    U8Str_Copy(&Entry->String, Label);
}

static inline void
DebugInspector_PushU32(debug_inspector* Inspector, u8_cstr Label, u32 Item)
{
    DebugInspector_PushEntry(Inspector, Label);
    U8Str_PushU32(&Entry->String, Item);
}

static inline void
DebugInspector_PushS32(debug_inspector* Inspector, u8_cstr Label, i32 Item)
{
    DebugInspector_PushEntry(Inspector, Label);
    U8Str_PushS32(&Entry->String, Item);
}

#endif //GAME_DEBUG_INSPECTOR_H
