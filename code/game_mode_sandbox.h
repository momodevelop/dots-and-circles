#ifndef GAME_MODE_SANDBOX
#define GAME_MODE_SANDBOX

#include "game.h"

// NOTE(Momo): Mode /////////////////////////////////////////////

struct anime_component {
    anime_id AnimeId;
    u32 CurrentFrameIndex;
    f32 TimerPerFrame;
    f32 DurationPerFrame;
};

static inline void
AnimeComponent_Create(anime_component* A,
                      anime_id Id, 
                      f32 DurationPerFrame) 
{
    A->AnimeId = Id;
    A->CurrentFrameIndex = 0;
    A->TimerPerFrame = 0.f;
    A->DurationPerFrame = DurationPerFrame;
}

static inline void
AnimeComponent_Update(anime_component* A, assets* Ass, f32 DeltaTime) {
    anime* Anime = Assets_GetAnime(Ass, Anime_KaruFront);
    A->TimerPerFrame += DeltaTime;
    
    A->TimerPerFrame += DeltaTime;
    if (A->TimerPerFrame >= 0.25f) 
    {
        A->TimerPerFrame = 0.f;
        ++A->CurrentFrameIndex;
        if (A->CurrentFrameIndex >= Anime->FrameCount) {
            A->CurrentFrameIndex = 0;
        }
    }
}

struct game_mode_sandbox_entity {
    v3f Pos;
    v3f Size;
    
    anime_component Anime;
};

struct game_mode_sandbox {
    game_mode_sandbox_entity Entity;
};

static inline void 
InitSandboxMode(permanent_state* PermState) {
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    Mode->Entity.Pos = V3f_Create(0.f, 0.f, 0.f);
    Mode->Entity.Size = V3f_Create(48.f*2, 48.f*2, 1.f);
    AnimeComponent_Create(&Mode->Entity.Anime, Anime_KaruFront, 0.25f);
    
}

static inline void
UpdateSandboxMode(permanent_state* PermState, 
                  transient_state* TranState,
                  mailbox* RenderCommands, 
                  game_input* Input,
                  f32 DeltaTime) 
{
    game_mode_sandbox* Mode = PermState->SandboxMode;     
    assets* Assets = &TranState->Assets;
    game_mode_sandbox_entity* Entity = &Mode->Entity;
    
    AnimeComponent_Update(&Entity->Anime, Assets, DeltaTime);
    
    // TODO(Momo): Draw function?
    m44f T = M44f_Translation(Entity->Pos.X, Entity->Pos.Y, Entity->Pos.Z);
    m44f S = M44f_Scale(Entity->Size.X, Entity->Size.Y, Entity->Size.Z);
    m44f Transform = M44f_Concat(T,S);
    c4f Color = Color_White;
    anime_component* AnimeCom = &Entity->Anime;
    {
        anime* Anime = Assets_GetAnime(Assets, AnimeCom->AnimeId);
        atlas_aabb_id AtlasAabbId = Anime->Frames[AnimeCom->CurrentFrameIndex];
        Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                       Assets,
                                       AtlasAabbId,
                                       Transform,
                                       Color);
        
    }
}



#endif 
