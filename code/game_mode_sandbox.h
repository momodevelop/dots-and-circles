#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX


// NOTE(Momo): Mode /////////////////////////////////////////////


struct game_mode_sandbox {
    v2f PrevMousePos;
};

static inline void 
InitSandboxMode(permanent_state* PermState) {
    
}




static inline void
UpdateSandboxMode(permanent_state* PermState, 
                  transient_state* TranState,
                  mailbox* RenderCommands, 
                  game_input* Input,
                  f32 DeltaTime) 
{
    
}


#endif 
