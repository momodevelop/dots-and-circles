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
    List<String_Buffer> Entries;
};

static inline void
DebugInspector_Init(debug_inspector* Inspector, Arena* arena) {
    Inspector->IsActive = false;
    List<String_Buffer>* Entries = &Inspector->Entries;
    Entries->alloc(arena, DebugInspector_EntryCount);
    for (u32 I = 0; I < DebugInspector_EntryCount; ++I) {
        String_Buffer* Item = Entries->push();
        alloc(Item, arena, DebugInspector_EntryCount);
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
        String_Buffer* Entry = Inspector->Entries + I;
        v3f Position = { DebugInspector_PosX, DebugInspector_PosY + OffsetY, DebugInspector_PosZ }; 
        Draw_Text(Font_Default, 
                  Position, 
                  Entry->str,
                  32.f, 
                  C4F_WHITE);
        clear(Entry);
        OffsetY -= 32.f;
    }
}

static inline String_Buffer*
DebugInspector_PushEntry(debug_inspector* Inspector, String Label) {
    String_Buffer* Entry = Inspector->Entries.push();
    copy(Entry, Label);
    return Entry;
}

static inline void
DebugInspector_PushU32(debug_inspector* Inspector, 
                       String Label, 
                       u32 Item)
{
    if(!Inspector->IsActive)
        return;
    String_Buffer* Entry = DebugInspector_PushEntry(Inspector, Label);
    push(Entry, Item);
}

static inline void
DebugInspector_PushS32(debug_inspector* Inspector, String Label, s32 Item)
{
    if(!Inspector->IsActive)
        return;
    
    String_Buffer* Entry = DebugInspector_PushEntry(Inspector, Label);
    push(Entry, Item);
}

#endif //GAME_DEBUG_INSPECTOR_H
