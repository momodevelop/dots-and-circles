/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H

#define DebugInspector_PosX -Game_DesignWidth * 0.5f + 10.f
#define DebugInspector_PosY Game_DesignHeight * 0.5f - 32.f
#define DebugInspector_PosZ 91.f
#define DebugInspector_EntryCount 32
#define DebugInspector_EntryLength 256

struct debug_inspector {
    b32 IsActive;
    MM_List<u8_str> Entries;
};

static inline void
DebugInspector_Init(debug_inspector* Inspector, MM_Arena* Arena) {
    Inspector->IsActive = False;
    MM_List<u8_str>* Entries = &Inspector->Entries;
    MM_List_New(Entries, Arena, DebugInspector_EntryCount);
    for (u32 I = 0; I < DebugInspector_EntryCount; ++I) {
        u8_str* Item = MM_List_Push(Entries);
        MM_U8Str_New(Item, Arena, DebugInspector_EntryCount);
    }
}

static inline void
DebugInspector_Begin(debug_inspector* Inspector) {
    if(!Inspector->IsActive)
        return;
    
    MM_List_Clear(&Inspector->Entries);
}

static inline void 
DebugInspector_End(debug_inspector* Inspector, 
                   mailbox* RenderCommands,
                   assets* Assets) {
    if(!Inspector->IsActive)
        return;
    
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < Inspector->Entries.count; ++I) {
        u8_str* Entry = Inspector->Entries + I;
        MM_V3f Position = MM_V3f_Create(DebugInspector_PosX, 
                                  DebugInspector_PosY + OffsetY,
                                  DebugInspector_PosZ); 
        Draw_Text(RenderCommands, 
                  Assets, 
                  Font_Default, 
                  Position, 
                  Entry->cstr,
                  32.f, 
                  Color_White);
        MM_U8Str_Clear(Entry);
        OffsetY -= 32.f;
    }
}

static inline u8_str*
DebugInspector_PushEntry(debug_inspector* Inspector, u8_cstr Label) {
    u8_str* Entry = MM_List_Push(&Inspector->Entries);
    MM_U8Str_CopyCStr(Entry, Label);
    return Entry;
}

static inline void
DebugInspector_PushU32(debug_inspector* Inspector, 
                       u8_cstr Label, 
                       u32 Item)
{
    if(!Inspector->IsActive)
        return;
    u8_str* Entry = DebugInspector_PushEntry(Inspector, Label);
    MM_U8Str_PushU32(Entry, Item);
}

static inline void
DebugInspector_PushS32(debug_inspector* Inspector, u8_cstr Label, s32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    u8_str* Entry = DebugInspector_PushEntry(Inspector, Label);
    MM_U8Str_PushS32(Entry, Item);
}

#endif //GAME_DEBUG_INSPECTOR_H
