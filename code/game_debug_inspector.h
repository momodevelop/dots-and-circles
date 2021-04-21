/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H

#define DebugInspector_PosX -Game_DesignWidth * 0.5f + 10.f
#define DebugInspector_PosY Game_DesignHeight * 0.5f - 32.f
#define DebugInspector_PosZ 91.f


typedef u8_fstr<256> debug_inspector_entry;

struct debug_inspector {
    b32 IsActive;
    flist<debug_inspector_entry,32> Entries;
};

static inline void
DebugInspector_Init(debug_inspector* Inspector) {
    Inspector->IsActive = False;
    
}

static inline void
DebugInspector_Begin(debug_inspector* Inspector) {
    if(!Inspector->IsActive)
        return;
    
    FList_Init(&Inspector->Entries);
}

static inline void 
DebugInspector_End(debug_inspector* Inspector, 
                   mailbox* RenderCommands,
                   assets* Assets) {
    if(!Inspector->IsActive)
        return;
    
    f32 OffsetY = 0.f;
    for (u32 I = 0; I < Inspector->Entries.Count; ++I) {
        debug_inspector_entry* Entry = FList_Get(&Inspector->Entries, I);
        v3f Position = V3f_Create(DebugInspector_PosX, 
                                  DebugInspector_PosY + OffsetY,
                                  DebugInspector_PosZ); 
        Draw_Text(RenderCommands, 
                  Assets, 
                  Font_Default, 
                  Position, 
                  U8FStr_ToCStr(Entry),
                  32.f, 
                  Color_White);
        U8FStr_Clear(Entry);
        OffsetY -= 32.f;
    }
}

static inline debug_inspector_entry*
DebugInspector_PushEntry(debug_inspector* Inspector, u8_cstr Label) {
    debug_inspector_entry* Entry = FList_Push(&Inspector->Entries, {});
    U8FStr_CopyCStr(Entry, Label);
    return Entry;
}

static inline void
DebugInspector_PushU32(debug_inspector* Inspector, u8_cstr Label, u32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    debug_inspector_entry* Entry = DebugInspector_PushEntry(Inspector, Label);
    U8FStr_PushU32(Entry, Item);
}

static inline void
DebugInspector_PushS32(debug_inspector* Inspector, u8_cstr Label, i32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    debug_inspector_entry* Entry = DebugInspector_PushEntry(Inspector, Label);
    U8FStr_PushS32(Entry, Item);
}

#endif //GAME_DEBUG_INSPECTOR_H
