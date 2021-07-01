/* date = April 24th 2021 4:53 pm */

#ifndef GAME_MODE_ANIME_TEST_H
#define GAME_MODE_ANIME_TEST_H

struct anime_component {
    Anime_ID AnimeId;
    u32 CurrentFrameIndex;
    f32 TimerPerFrame;
    f32 DurationPerFrame;
};

static inline void
AnimeComponent_Create(anime_component* A,
                      Anime_ID Id, 
                      f32 DurationPerFrame) 
{
    A->AnimeId = Id;
    A->CurrentFrameIndex = 0;
    A->TimerPerFrame = 0.f;
    A->DurationPerFrame = DurationPerFrame;
}

static inline void
AnimeComponent_Update(anime_component* A, assets* Ass, f32 DeltaTime) {
    anime* Anime = Assets_GetAnime(Ass, ANIME_KARU_FRONT);
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

struct game_mode_anime_test_entity {
    v3f Pos;
    v3f Size;
    
    anime_component Anime;
};

struct game_mode_anime_test {
    game_mode_anime_test_entity Entity;
    v2f PrevMousePos;
};

static inline void 
AnimeTestMode_Init(permanent_state* PermState) {
    game_mode_anime_test* Mode = PermState->AnimeTestMode;     
    Mode->Entity.Pos = v3f::create(0.f, 0.f, 0.f);
    Mode->Entity.Size = v3f::create(48.f*2, 48.f*2, 1.f);
    AnimeComponent_Create(&Mode->Entity.Anime, ANIME_KARU_FRONT, 0.25f);
    
}




static inline void
AnimeTestMode_Update(permanent_state* PermState, 
                     transient_state* TranState,
                     f32 DeltaTime) 
{
    game_mode_anime_test* Mode = PermState->AnimeTestMode;     
    assets* Assets = &TranState->Assets;
    game_mode_anime_test_entity* Entity = &Mode->Entity;
    
    AnimeComponent_Update(&Entity->Anime, Assets, DeltaTime);
    
    // TODO(Momo): Draw function?
    m44f T = m44f::create_translation(Entity->Pos.x, Entity->Pos.y, Entity->Pos.z);
    m44f S = m44f::create_scale(Entity->Size.x, Entity->Size.y, Entity->Size.z);
    
    c4f Color = C4F_WHITE;
    anime_component* AnimeCom = &Entity->Anime;
    {
        anime* Anime = Assets_GetAnime(Assets, AnimeCom->AnimeId);
        Image_ID ImageId = Anime->Frames[AnimeCom->CurrentFrameIndex];
        Draw_TexturedQuadFromImage(ImageId,
                                   T*S,
                                   Color);
        
    }
    
}


#endif //GAME_MODE_ANIME_TEST_H
