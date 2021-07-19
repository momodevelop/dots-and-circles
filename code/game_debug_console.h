#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__


typedef void (*Console_Callback)(struct Console* console, void* context, String args);

struct Console_Command {
    String key;
    Console_Callback callback;
    void* context;
};

struct Console_Line {
    StringBuffer text;
    c4f color;
};

struct Console {
    b8 is_active;
    
    v3f dimensions;
    v3f position;
    v3f start_position;
    v3f end_position;
    
    Array<Console_Line> info_lines;
    Console_Line input_line;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    Timer start_pop_repeat_timer;
    Timer pop_repeat_timer;
    b8 is_start_pop;
    
    // Enter and Exit transitions for swag!
    Timer transit_timer;
    
    // List of commands
    List<Console_Command> commands;
};


static inline b8
Console_Init(Console* c, Arena* arena, u32 max_commands)
{
    const u32 line_length = 110;
    const u32 info_line_count = 5;
    const f32 z_position = 90.f;
    
    c->transit_timer = Timer::create(0.25f);
    
    c->dimensions = v3f_Create(GAME_DESIGN_WIDTH, 240.f, 1.f);
    c->start_position = v3f_Create(0.f, 
                                   -GAME_DESIGN_HEIGHT/2 - c->dimensions.h/2,
                                   z_position);
    c->end_position = v3f_Create(0.f, 
                                 -GAME_DESIGN_HEIGHT/2 + c->dimensions.h/2,
                                 z_position);
    c->position = v3f_Create();
    
    // NOTE(Momo): Timers related to type
    c->start_pop_repeat_timer = Timer::create(0.5f);
    c->pop_repeat_timer = Timer::create(0.1f); 
    
    if (!List_Alloc(&c->commands, arena, max_commands)) {
        return false;
    }
    
    if (!StringBuffer_Alloc(&c->input_line.text, arena, line_length)) {
        return false;
    }
    
    if (!Array_Alloc(&c->info_lines, arena, 5)) {
        return false;
    }
    for (u32 i = 0; i < c->info_lines.count; ++i) {
        if (!StringBuffer_Alloc(&c->info_lines[i].text, arena, line_length)){
            return false;
        }
    }
    
    return true;
}

static inline b8 
Console_AddCommand(Console* c, String key, 
                   Console_Callback callback, 
                   void* context)
{
    Console_Command* command = List_Push(&c->commands);
    if (command == nullptr) {
        return false;
    }
    command->key = key;
    command->callback = callback;
    command->context = context;
    
    return true;
}



static inline void
Console_PushInfo(Console* c, String str, c4f color) {
    for (u32 i = 0; i < c->info_lines.count - 1; ++i) {
        u32 j = c->info_lines.count - 1 - i;
        Console_Line* dest = c->info_lines + j;
        Console_Line* src = c->info_lines + j - 1;
        StringBuffer_Copy(&dest->text, src->text.str);
        dest->color = src->color;
    }
    c->info_lines[0].color = color;
    StringBuffer_Clear(&c->info_lines[0].text);
    StringBuffer_Copy(&c->info_lines[0].text, str);
}


static inline void 
Console_Pop(Console* c) {
    StringBuffer_Pop(&c->input_line.text);
}

static inline void 
Console_RemoveCommand(Console* c, String key) {
    for (u32 i = 0; i < c->commands.count; ++i) {
        if (c->commands[i].key == key) {
            List_Slear(&c->commands, i);
            return;
        }
    }
    
}

// Returns true if there is a new command
static inline void 
Console_Update(Console* c, f32 dt) 
{
    if (g_input->button_console.is_poked()) {
        c->is_active = !c->is_active; 
    }
    
    // Transition
    {
        f32 p = EaseInQuad(c->transit_timer.percent());
        v3f delta = (c->end_position - c->start_position) * p; 
        c->position = c->start_position + delta; 
    }
    
    if (c->is_active) {
        c->transit_timer.tick(dt);
        if (g_input->characters.count > 0 && 
            g_input->characters.count <= StringBuffer_Remaining(&c->input_line.text))
        {  
            StringBuffer_Push(&c->input_line.text, g_input->characters.str);
        }
        
        // Remove character backspace logic
        if (g_input->button_back.is_down()) {
            if(!c->is_start_pop) {
                Console_Pop(c);
                c->is_start_pop = true;
                c->start_pop_repeat_timer.reset();
                c->pop_repeat_timer.reset();
            }
            else {
                if (c->start_pop_repeat_timer.is_end()) {
                    if(c->pop_repeat_timer.is_end()) {
                        Console_Pop(c);
                        c->pop_repeat_timer.reset();
                    }
                    c->pop_repeat_timer.tick(dt);
                }
                c->start_pop_repeat_timer.tick(dt);
            }
        }
        else {
            c->is_start_pop = false; 
        }
        
        // Execute command
        String input_line_str = c->input_line.text.str;
        if (g_input->button_confirm.is_poked()) {
            Console_PushInfo(c, input_line_str, C4F_WHITE);
            
            u32 min = 0;
            u32 max = 0;
            for (u32 i = 0; i < c->input_line.text.count; ++i) {
                if (c->input_line.text.data[i] == ' ') {
                    max = i;
                    break;
                }
            }
            String command_str = String_Create(input_line_str, min, max);
            
            // Send a command to a callback
            for (u32 i = 0; i < c->commands.count; ++i) {
                Console_Command* command = c->commands + i;
                if (command->key == command_str) {
                    command->callback(c, 
                                      command->context, 
                                      input_line_str);
                }
            }
            
            StringBuffer_Clear(&c->input_line.text);
            
        }
    }
    else if (!c->is_active) {
        c->transit_timer.untick(dt);
    }
    
    
}


static inline void
Console_Render(Console* c) 
{
    if (c->transit_timer.is_begin()) {
        return;
    }
    
    Font* font= Assets_GetFont(g_assets, FONT_DEFAULT);
    f32 bottom = c->position.y - c->dimensions.h * 0.5f;
    f32 left = c->position.x - c->dimensions.w * 0.5f;
    f32 line_height = c->dimensions.h / (c->info_lines.count + 1);
    f32 font_size = line_height * 0.9f;
    f32 font_height = Font_Height(font) * font_size;
    
    f32 padding_h = (line_height - font_height) * 0.5f  + Abs(font->descent) * font_size; 
    f32 padding_w = c->dimensions.w * 0.005f;
    {
        m44f s = m44f_Scale(c->dimensions);
        m44f p = m44f_Translation(c->position);
        Renderer_DrawQuad(g_renderer, C4F_GREY3, p*s);
    }
    
    {
        m44f s = m44f_Scale(c->dimensions.w, line_height, 0.f);
        m44f p = m44f_Translation(c->position.x, 
                                  bottom + line_height * 0.5f,
                                  c->position.z + 0.01f);
        
        Renderer_DrawQuad(g_renderer, C4F_GREY2, p*s);
    }
    
    // Draw info text
    {
        for (u32 i = 0; i < c->info_lines.count ; ++i) {
            v3f text_position = {};
            text_position.x = left + padding_w;
            text_position.y = bottom + ((i+1) * line_height) + padding_h;
            text_position.z = c->position.z + 0.01f;
            
            String infoLineCStr = c->info_lines[i].text.str;
            DrawText(FONT_DEFAULT, 
                     text_position,
                     c->info_lines[i].text.str,
                     font_size,
                     c->info_lines[i].color);
        }
        
        v3f text_position = {};
        text_position.x = left + padding_w;
        text_position.y = bottom + padding_h;
        text_position.z = c->position.z + 0.02f;
        
        DrawText(FONT_DEFAULT, 
                 text_position,
                 c->input_line.text.str,
                 font_size,
                 C4F_WHITE);
        
        
    }
}





#endif
