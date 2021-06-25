/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H

#define DebugInspector_PosX -Game_DesignWidth * 0.5f + 10.f
#define DebugInspector_PosY Game_DesignHeight * 0.5f - 32.f
#define DebugInspector_PosZ 91.f
#define DebugInspector_EntryCount 32
#define DebugInspector_EntryLength 256

struct debug_inspector {
    b8 IsActive;
    List<u8_str> Entries;
};

static inline void
DebugInspector_Init(debug_inspector* Inspector, Arena* arena) {
    Inspector->IsActive = false;
    List<u8_str>* Entries = &Inspector->Entries;
    Entries->alloc(arena, DebugInspector_EntryCount);
    for (u32 I = 0; I < DebugInspector_EntryCount; ++I) {
        u8_str* Item = Entries->push();
        U8Str_New(Item, arena, DebugInspector_EntryCount);
    }
}

static inline void
DebugInspector_Begin(debug_inspector* Inspector) {
    if(!Inspector->IsActive)
        return;
    
    Inspector->Entries.clear();
}

static inline void 
DebugInspector_End(debug_inspector* Inspector) 
{
    if(!Inspector->IsActive)
        return;
    
    f32 OffsetY = 0.f;
    
    for (u32 I = 0; I < Inspector->Entries.count; ++I) {
        u8_str* Entry = Inspector->Entries + I;
        v3f Position = V3f_Create(DebugInspector_PosX, 
                                  DebugInspector_PosY + OffsetY,
                                  DebugInspector_PosZ); 
        Draw_Text(Font_Default, 
                  Position, 
                  Entry->CStr,
                  32.f, 
                  C4f_White);
        U8Str_Clear(Entry);
        OffsetY -= 32.f;
    }
}

static inline u8_str*
DebugInspector_PushEntry(debug_inspector* Inspector, u8_cstr Label) {
    u8_str* Entry = Inspector->Entries.push();
    U8Str_CopyCStr(Entry, Label);
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
    U8Str_PushU32(Entry, Item);
}

static inline void
DebugInspector_PushS32(debug_inspector* Inspector, u8_cstr Label, s32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    u8_str* Entry = DebugInspector_PushEntry(Inspector, Label);
    U8Str_PushS32(Entry, Item);
}

#endif //GAME_DEBUG_INSPECTOR_H
