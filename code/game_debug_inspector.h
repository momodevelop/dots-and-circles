/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H


struct Inspector {
    b8 is_active;
    List<StringBuffer> entries;
};

b8
Inspector_Init(Inspector* in, Arena* arena) {
    constexpr static u32 entry_count = 32;
    
    in->is_active = false;
    if (!List_Alloc(&in->entries, arena, entry_count)) {
        return false;
    }
    
    for (u32 i = 0; i < entry_count; ++i) {
        StringBuffer* item = List_Push(&in->entries);
        if (!StringBuffer_Alloc(item, arena, entry_count)) {
            return false;
        }
    }
    
    return true;
}

void
Inspector_Begin(Inspector* in) {
    if(!in->is_active)
        return;
    
    List_Clear(&in->entries);
}

void 
Inspector_End(Inspector* in) 
{
    if(!in->is_active)
        return;
    
    f32 offset_y = 0.f;
    
    for (u32 i = 0; i < in->entries.count; ++i) {
        StringBuffer* entry = in->entries + i;
        v3f position = { 
            -GAME_DESIGN_WIDTH * 0.5f + 10.f, 
            (GAME_DESIGN_HEIGHT * 0.5f - 32.f) + offset_y, 
            91.f
        }; 
        
        
        DrawText(FONT_DEFAULT, 
                 position, 
                 entry->str,
                 32.f, 
                 C4F_WHITE);
        StringBuffer_Clear(entry);
        offset_y -= 32.f;
    }
}

StringBuffer*
Inspector_PushEntry(Inspector* in, String label) {
    StringBuffer* entry = List_Push(&in->entries);
    StringBuffer_Copy(entry, label);
    return entry;
}

void
Inspector_Push(Inspector* in, 
               String label, 
               u32 item)
{
    if(!in->is_active)
        return;
    StringBuffer* entry = Inspector_PushEntry(in, label);
    StringBuffer_Push(entry, item);
}

void
Inspector_Push(Inspector* in, String label, s32 item)
{
    if(!in->is_active)
        return;
    
    StringBuffer* entry = Inspector_PushEntry(in, label);
    StringBuffer_Push(entry, item);
}

#endif //GAME_DEBUG_INSPECTOR_H
