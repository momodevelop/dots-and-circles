


#include "game.h"

// cmd: jump main/menu/atlas_test/etc...
static inline void 
cmd_jump(Debug_Console* console, void* context, String args) {
    auto* debug_state = (Debug_State*)context;
    Permanent_State* perm_state = debug_state->perm_state;
    
    defer{ g_scratch->clear(); };
    
    String buffer = {};
    String_Split_Result arg_list = args.split(g_scratch, ' ');
    if ( arg_list.item_count != 2 ) {
        // Expect two arguments
        buffer.init("Expected only 2 arguments");
        console->push_info(buffer, 
                           C4F_RED);
        return;
    }
    
    String state_to_chage_to = arg_list.items[1];
    if (state_to_chage_to == "main") {
        buffer.init("Jumping to Main");
        console->push_info(buffer, 
                           C4F_YELLOW);
        perm_state->next_game_mode = GAME_MODE_MAIN;
    }
    else if (state_to_chage_to == "splash") {
        buffer.init("Jumping to Splash");
        console->push_info(buffer,  
                           C4F_YELLOW);
        perm_state->next_game_mode = GAME_MODE_SPLASH;
    }
    else if (state_to_chage_to == "sandbox") {
        buffer.init("Jumping to Sandbox");
        console->push_info(buffer, 
                           C4F_YELLOW);
        perm_state->next_game_mode = GAME_MODE_SANDBOX;
    }
    else if (state_to_chage_to == "anime") {
        buffer.init("Jumping to Anime");
        console->push_info(buffer, 
                           C4F_YELLOW);
        perm_state->next_game_mode = GAME_MODE_ANIME_TEST;
    }
    else {
        buffer.init("Invalid state to jump to");
        console->push_info(buffer, 
                           C4F_RED);
    }
}

extern "C" 
GameUpdateFunc(game_update) 
{
    // NOTE(Momo): Initialize globals
    Arena scratch = {};
    {
        // Let's say we want to time this block
        g_platform = platform_api;
        g_log = platform_api->log;
        g_renderer = render_commands;
        g_input = platform_input;
        if (!scratch.init(game_memory->scratch_memory, game_memory->scratch_memory_size)) {
            g_log("Cannot initialize Scratch Memory");
            return false;
        }
        g_scratch = &scratch;
    }
    
    
    
    auto* perm_state = (Permanent_State*)game_memory->permanent_memory;
    auto* tran_state = (Transient_State*)game_memory->transient_memory;
    auto* debug_state = (Debug_State*)game_memory->debug_memory; 
    
    //  Initialization of the game
    if(!perm_state->is_initialized) {
        g_log("[Permanent] Init Begin\n");
        // NOTE(Momo): Arenas
        perm_state = ARENA_BOOT_STRUCT(Permanent_State,
                                       arena,
                                       game_memory->permanent_memory, 
                                       game_memory->permanent_memory_size);
        
        perm_state->mode_arena = perm_state->arena.mark();
        perm_state->current_game_mode = GAME_MODE_NONE;
        perm_state->next_game_mode = GAME_MODE_MAIN;
        perm_state->is_initialized = true;
        perm_state->is_paused = false;
        
        Renderer_SetDesignResolution(g_renderer,
                                     GAME_DESIGN_WIDTH, 
                                     GAME_DESIGN_HEIGHT);
        perm_state->game_speed = 1.f;
        g_log("[Permanent] Init End\n");
        
    }
    
    
    
    if (!tran_state->is_initialized) {
        g_log("[Transient] Init Begin\n");
        tran_state = ARENA_BOOT_STRUCT(Transient_State,
                                       arena,
                                       game_memory->transient_memory, 
                                       game_memory->transient_memory_size);
        b8 success = tran_state->assets.init(&tran_state->arena);
        
        if(!success) {
            g_log("[Transient] Failed to initialize assets\n");
            return false;
        }
        g_assets = &tran_state->assets;
        
        success =  tran_state->mixer.init(1.f, 32, &tran_state->arena);
        if (!success) {
            g_log("[Transient] Failed to initialize audio\n");
            return false;
        }
        
        tran_state->is_initialized = true;
        g_log("[Transient] Init End\n");
    }
    
    if (!debug_state->is_initialized) {
        g_log("[Debug] Init Begin\n");
        debug_state = ARENA_BOOT_STRUCT(Debug_State,
                                        arena,
                                        game_memory->debug_memory,
                                        game_memory->debug_memory_size);
        // Init inspector
        debug_state->inspector.init(&debug_state->arena);
        
        
        // Init console
        {
            String buffer = {};
            buffer.init("jump");
            
            if (!debug_state->console.init(&debug_state->arena, 16)) {
                return false;
            }
            
            if (!debug_state->console.add_command(buffer, 
                                                  cmd_jump, 
                                                  debug_state)) {
                return false;
            }
        }
        
        debug_state->perm_state = perm_state;
        debug_state->tran_state = tran_state;
        debug_state->is_initialized = true;
        g_log("[Debug] Init End\n");
    }
    
    // NOTE(Momo): Input
    // TODO(Momo): Consider putting everything into a Debug_Update()
    // Or, change seperate variable state into inspector and update seperately
    if (g_input->button_inspector.is_poked()) {
        debug_state->inspector.is_active = !debug_state->inspector.is_active;
    }
    //START_PROFILING(Test);
    debug_state->inspector.begin();
    debug_state->console.update(dt);
    //END_PROFILING(Test);
    
    
    // NOTE(Momo): Pause
    if (g_input->button_pause.is_poked()) {
        perm_state->is_paused = !perm_state->is_paused;
    }
    if (perm_state->is_paused) {
        dt = 0.f;
    }
    
    // NOTE(Momo): speed up/down
    if (g_input->button_speed_down.is_poked()) {
        perm_state->game_speed -= 0.1f;
    }
    if (g_input->button_speed_up.is_poked()) {
        perm_state->game_speed += 0.1f;
    }
    dt *= perm_state->game_speed;
    
    
    // NOTE(Momo): Clean state/Switch states
    if (perm_state->next_game_mode != GAME_MODE_NONE) {
        perm_state->mode_arena.revert();
        Arena* mode_arena = perm_state->mode_arena;
        
        switch(perm_state->next_game_mode) {
            case GAME_MODE_SPLASH: {
                perm_state->splash_mode = mode_arena->push_struct<Game_Mode_Splash>(); 
                init_splash_mode(perm_state);
            } break;
            case GAME_MODE_MAIN: {
                perm_state->main_mode = mode_arena->push_struct<Game_Mode_Main>(); 
                if (!init_main_mode(perm_state, tran_state, debug_state)){
                    return false;
                }
            } break;
            case GAME_MODE_SANDBOX: {
                perm_state->sandbox_mode = mode_arena->push_struct<Game_Mode_Sandbox>(); 
                init_sandbox_mode(perm_state);
            } break;
            case GAME_MODE_ANIME_TEST: {
                perm_state->anime_test_mode = mode_arena->push_struct<Game_Mode_Anime_Test>(); 
                init_anime_test_mode(perm_state);
            } break;
            default: {
                return false;
            }
        }
        
        perm_state->current_game_mode = perm_state->next_game_mode;
        perm_state->next_game_mode = GAME_MODE_NONE;
    }
    
    String buffer = {};
    buffer.init("Debug Memory: ");
    debug_state->inspector.push_u32(buffer,
                                    (u32)debug_state->arena.remaining());
    buffer.init("Mode Memory: ");
    debug_state->inspector.push_u32(buffer,
                                    (u32)perm_state->mode_arena.arena->remaining());
    buffer.init("Trans Memory: ");
    debug_state->inspector.push_u32(buffer,
                                    (u32)tran_state->arena.remaining());
    
    
    // State update
    switch(perm_state->current_game_mode) {
        case GAME_MODE_SPLASH: {
            update_splash_mode(perm_state, 
                               tran_state,
                               dt);
        } break;
        case GAME_MODE_MAIN: {
            update_main_mode(perm_state, 
                             tran_state,
                             debug_state,
                             dt);
        } break; 
        case GAME_MODE_SANDBOX: {
            update_sandbox_mode(perm_state, 
                                tran_state,
                                dt);
        } break;
        case GAME_MODE_ANIME_TEST: {
            update_anime_test_mode(perm_state, 
                                   tran_state,
                                   dt);
        } break;
        default: {
            return false;
        }
    }
    
    debug_state->console.render();
    debug_state->inspector.end();
    
    tran_state->mixer.update(platform_audio);
    
    
    return true;
}