/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H


struct Debug_Inspector {
    b8 is_active;
    List<StringBuffer> entries;
    
    inline b8 init(Arena* arena);
    inline void begin();
    inline void end();
    inline StringBuffer* push_entry(String label);
    inline void push_u32(String label, u32 item);
    inline void push_s32(String label, s32 item);
};

b8
Debug_Inspector::init(Arena* arena) {
    constexpr static u32 entry_count = 32;
    
    this->is_active = false;
    if (!List_Alloc(&this->entries, arena, entry_count)) {
        return false;
    }
    
    for (u32 i = 0; i < entry_count; ++i) {
        StringBuffer* item = List_Push(&this->entries);
        if (!StringBuffer_Alloc(item, arena, entry_count)) {
            return false;
        }
    }
    
    return true;
}

void
Debug_Inspector::begin() {
    if(!this->is_active)
        return;
    
    List_Clear(&this->entries);
}

void 
Debug_Inspector::end() 
{
    if(!this->is_active)
        return;
    
    f32 offset_y = 0.f;
    
    for (u32 I = 0; I < this->entries.count; ++I) {
        StringBuffer* entry = this->entries + I;
        v3f position = { 
            -GAME_DESIGN_WIDTH * 0.5f + 10.f, 
            (GAME_DESIGN_HEIGHT * 0.5f - 32.f) + offset_y, 
            91.f
        }; 
        
        
        draw_text(FONT_DEFAULT, 
                  position, 
                  entry->str,
                  32.f, 
                  C4F_WHITE);
        StringBuffer_Clear(entry);
        offset_y -= 32.f;
    }
}

StringBuffer*
Debug_Inspector::push_entry(String label) {
    StringBuffer* entry = List_Push(&this->entries);
    StringBuffer_Copy(entry, label);
    return entry;
}

void
Debug_Inspector::push_u32(String label, 
                          u32 item)
{
    if(!this->is_active)
        return;
    StringBuffer* entry = this->push_entry(label);
    StringBuffer_Push(entry, item);
}

void
Debug_Inspector::push_s32(String label, s32 item)
{
    if(!this->is_active)
        return;
    
    StringBuffer* entry = this->push_entry(label);
    StringBuffer_Push(entry, item);
}

#endif //GAME_DEBUG_INSPECTOR_H
