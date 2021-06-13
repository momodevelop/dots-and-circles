/* date = April 24th 2021 4:53 pm */

#ifndef GAME_MODE_ANIME_TEST_H
#define GAME_MODE_ANIME_TEST_H

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
InitAnimeTestMode(permanent_state* PermState) {
    game_mode_anime_test* Mode = PermState->AnimeTestMode;     
    Mode->Entity.Pos = V3f_Create(0.f, 0.f, 0.f);
    Mode->Entity.Size = V3f_Create(48.f*2, 48.f*2, 1.f);
    AnimeComponent_Create(&Mode->Entity.Anime, Anime_KaruFront, 0.25f);
    
}




static inline void
UpdateAnimeTestMode(permanent_state* PermState, 
                    transient_state* TranState,
                    f32 DeltaTime) 
{
    game_mode_anime_test* Mode = PermState->AnimeTestMode;     
    assets* Assets = &TranState->Assets;
    game_mode_anime_test_entity* Entity = &Mode->Entity;
    
    AnimeComponent_Update(&Entity->Anime, Assets, DeltaTime);
    
    // TODO(Momo): Draw function?
    m44f T = M44f_Translation(Entity->Pos.X, Entity->Pos.Y, Entity->Pos.Z);
    m44f S = M44f_Scale(Entity->Size.X, Entity->Size.Y, Entity->Size.Z);
    m44f Transform = M44f_Concat(T,S);
    c4f Color = C4f_White;
    anime_component* AnimeCom = &Entity->Anime;
    {
        anime* Anime = Assets_GetAnime(Assets, AnimeCom->AnimeId);
        image_id ImageId = Anime->Frames[AnimeCom->CurrentFrameIndex];
        Draw_TexturedQuadFromImage(ImageId,
                                   Transform,
                                   Color);
        
    }
    
}


#endif //GAME_MODE_ANIME_TEST_H
