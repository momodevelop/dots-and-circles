#ifndef __GAME_CONSOLE__
#define __GAME_CONSOLE__


typedef void (*Debug_Console_Callback)(struct Debug_Console* console, void* context, String args);

struct Debug_Console_Command {
    String key;
    Debug_Console_Callback callback;
    void* context;
};

struct Debug_Console_Line {
    String_Buffer text;
    c4f color;
};

struct Debug_Console {
    //- Variables 
    b8 is_active;
    
    v3f dimensions;
    v3f position;
    v3f start_position;
    v3f end_position;
    
    Array<Debug_Console_Line> info_lines;
    Debug_Console_Line input_line;
    
    // Backspace (to delete character) related
    // Maybe make an easing system?
    Timer start_pop_repeat_timer;
    Timer pop_repeat_timer;
    b8 is_start_pop;
    
    // Enter and Exit transitions for swag!
    Timer transit_timer;
    
    // List of commands
    List<Debug_Console_Command> commands;
    
    
    //- Functions
    b8 init(Arena* arena, u32 max_commands);
    b8 add_command(String key, Debug_Console_Callback callback, void* context);
    void push_info(String str, c4f color);
    void pop();
    void update(f32 dt);
    void render();
    void remove_command(String key);
};


b8
Debug_Console::init(Arena* arena, u32 max_commands)
{
    const u32 line_length = 110;
    const u32 info_line_count = 5;
    const f32 z_position = 90.f;
    
    transit_timer = Timer::create(0.25f);
    
    dimensions = v3f::create(GAME_DESIGN_WIDTH, 240.f, 1.f);
    start_position = v3f::create(0.f, 
                                 -GAME_DESIGN_HEIGHT/2 - dimensions.h/2,
                                 z_position);
    end_position = v3f::create(0.f, 
                               -GAME_DESIGN_HEIGHT/2 + dimensions.h/2,
                               z_position);
    position = v3f::create();
    
    // NOTE(Momo): Timers related to type
    start_pop_repeat_timer = Timer::create(0.5f);
    pop_repeat_timer = Timer::create(0.1f); 
    
    if (!commands.alloc(arena, max_commands)) {
        return false;
    }
    
    if (!input_line.text.alloc(arena, line_length)) {
        return false;
    }
    
    if (!info_lines.alloc(arena, 5)) {
        return false;
    }
    for (u32 i = 0; i < info_lines.count; ++i) {
        if (!info_lines[i].text.alloc(arena, line_length)){
            return false;
        }
    }
    
    return true;
}

b8 
Debug_Console::add_command(String key, 
                           Debug_Console_Callback callback, 
                           void* context)
{
    Debug_Console_Command* command = commands.push();
    if (command == nullptr) {
        return false;
    }
    command->key = key;
    command->callback = callback;
    command->context = context;
    
    return true;
}



void
Debug_Console::push_info(String str, c4f color) {
    for (u32 i = 0; i < info_lines.count - 1; ++i) {
        u32 j = info_lines.count - 1 - i;
        Debug_Console_Line* dest = info_lines + j;
        Debug_Console_Line* src = info_lines + j - 1;
        dest->text.copy(src->text.str);
        dest->color = src->color;
    }
    info_lines[0].color = color;
    info_lines[0].text.clear();
    info_lines[0].text.copy(str);
}


void 
Debug_Console::pop() {
    input_line.text.pop();
}

void 
Debug_Console::remove_command(String key) {
    for (u32 i = 0; i < commands.count; ++i) {
        if (commands[i].key == key) {
            commands.slear(i);
            return;
        }
    }
    
}

// Returns true if there is a new command
void 
Debug_Console::update(f32 dt) 
{
    if (g_input->button_console.is_poked()) {
        is_active = !is_active; 
    }
    
    // Transition
    {
        f32 p = ease_in_quad(transit_timer.percent());
        v3f delta = (end_position - start_position) * p; 
        position = start_position + delta; 
    }
    
    if (is_active) {
        transit_timer.tick(dt);
        if (g_input->characters.count > 0 && 
            g_input->characters.count <= input_line.text.remaining()) 
        {  
            input_line.text.push(g_input->characters.str);
        }
        
        // Remove character backspace logic
        if (g_input->button_back.is_down()) {
            if(!is_start_pop) {
                pop();
                is_start_pop = true;
                start_pop_repeat_timer.reset();
                pop_repeat_timer.reset();
            }
            else {
                if (start_pop_repeat_timer.is_end()) {
                    if(pop_repeat_timer.is_end()) {
                        pop();
                        pop_repeat_timer.reset();
                    }
                    pop_repeat_timer.tick(dt);
                }
                start_pop_repeat_timer.tick(dt);
            }
        }
        else {
            is_start_pop = false; 
        }
        
        // Execute command
        String input_line_str = input_line.text.str;
        if (g_input->button_confirm.is_poked()) {
            push_info(input_line_str, C4F_WHITE);
            
            u32 min = 0;
            u32 max = 0;
            for (u32 i = 0; i < input_line.text.count; ++i) {
                if (input_line.text.data[i] == ' ') {
                    max = i;
                    break;
                }
            }
            String command_str = {};
            command_str.init(input_line_str, 
                             min, 
                             max); 
            
            // Send a command to a callback
            for (u32 i = 0; i < commands.count; ++i) {
                Debug_Console_Command* command = commands + i;
                if (command->key == command_str) {
                    command->callback(this, 
                                      command->context, 
                                      input_line_str);
                }
            }
            input_line.text.clear();
            
        }
    }
    else if (!is_active) {
        transit_timer.untick(dt);
    }
    
    
}


void
Debug_Console::render() 
{
    if (transit_timer.is_begin()) {
        return;
    }
    
    Font* font= g_assets->get_font(FONT_DEFAULT);
    f32 bottom = position.y - dimensions.h * 0.5f;
    f32 left = position.x - dimensions.w * 0.5f;
    f32 line_height = dimensions.h / (info_lines.count + 1);
    f32 font_size = line_height * 0.9f;
    f32 font_height = font->height() * font_size;
    
    f32 padding_h = (line_height - font_height) * 0.5f  + ABS(font->descent) * font_size; 
    f32 padding_w = dimensions.w * 0.005f;
    {
        m44f s = m44f::create_scale(dimensions);
        m44f p = m44f::create_translation(position);
        Renderer_DrawQuad(g_renderer, 
                          C4F_GREY3, 
                          p* s);
    }
    
    {
        m44f s = m44f::create_scale(dimensions.w, line_height, 0.f);
        m44f p = m44f::create_translation(position.x, 
                                          bottom + line_height * 0.5f,
                                          position.z + 0.01f);
        
        
        Renderer_DrawQuad(g_renderer, 
                          C4F_GREY2, 
                          p* s);
    }
    
    // Draw info text
    {
        for (u32 i = 0; i < info_lines.count ; ++i) {
            v3f text_position = {};
            text_position.x = left + padding_w;
            text_position.y = bottom + ((i+1) * line_height) + padding_h;
            text_position.z = position.z + 0.01f;
            
            String infoLineCStr = info_lines[i].text.str;
            draw_text(FONT_DEFAULT, 
                      text_position,
                      info_lines[i].text.str,
                      font_size,
                      info_lines[i].color);
        }
        
        v3f text_position = {};
        text_position.x = left + padding_w;
        text_position.y = bottom + padding_h;
        text_position.z = position.z + 0.02f;
        
        draw_text(FONT_DEFAULT, 
                  text_position,
                  input_line.text.str,
                  font_size,
                  C4F_WHITE);
        
        
    }
}





#endif
