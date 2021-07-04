/* date = March 24th 2021 10:04 pm */

#ifndef GAME_DEBUG_INSPECTOR_H
#define GAME_DEBUG_INSPECTOR_H


struct Debug_Inspector {
    b8 is_active;
    List<String_Buffer> entries;
    
    b8 init(Arena* arena);
    void begin();
    void end();
    String_Buffer* push_entry(String label);
    void push_u32(String label, u32 item);
    void push_s32(String label, s32 item);
};

b8
Debug_Inspector::init(Arena* arena) {
    constexpr static u32 entry_count = 32;
    
    this->is_active = false;
    if (!this->entries.alloc(arena, entry_count)) {
        return false;
    }
    
    for (u32 i = 0; i < entry_count; ++i) {
        String_Buffer* item = this->entries.push();
        if (!item->alloc(arena, entry_count)) {
            return false;
        }
    }
    
    return true;
}

void
Debug_Inspector::begin() {
    if(!this->is_active)
        return;
    
    this->entries.clear();
}

void 
Debug_Inspector::end() 
{
    if(!this->is_active)
        return;
    
    f32 offset_y = 0.f;
    
    for (u32 I = 0; I < this->entries.count; ++I) {
        String_Buffer* entry = this->entries + I;
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
        entry->clear();
        offset_y -= 32.f;
    }
}

String_Buffer*
Debug_Inspector::push_entry(String label) {
    String_Buffer* entry = this->entries.push();
    entry->copy(label);
    return entry;
}

void
Debug_Inspector::push_u32(String label, 
                          u32 item)
{
    if(!this->is_active)
        return;
    String_Buffer* entry = this->push_entry(label);
    entry->push(item);
}

void
Debug_Inspector::push_s32(String label, s32 item)
{
    if(!this->is_active)
        return;
    
    String_Buffer* entry = this->push_entry(label);
    entry->push(item);
}

#endif //GAME_DEBUG_INSPECTOR_H
