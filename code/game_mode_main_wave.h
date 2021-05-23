/* date = May 22nd 2021 7:12 pm */

#ifndef GAME_MODE_MAIN_WAVE_H
#define GAME_MODE_MAIN_WAVE_H

static inline void
UpdateWaves(game_mode_main* Mode, 
            assets* Assets,
            f32 DeltaTime) 
{
    if (Mode->Wave.IsDone) {
        // TODO: Random wave type
        Mode->Wave.Type = WavePatternType_SpawnNForDuration;
        // Initialize the wave
        switch (Mode->Wave.Type) {
            case WavePatternType_SpawnNForDuration: {
                auto* Pattern = &Mode->Wave.PatternSpawnNForDuration;
                Pattern->EnemiesPerSpawn = 1;
                Pattern->SpawnTimer = 0.f;
                Pattern->SpawnDuration = 3.f;
                Pattern->Timer = 0.f;
                Pattern->Duration = 30.f;
            } break;
            default: {
                Assert(false);
            }
        }
        Mode->Wave.IsDone = false;
    }
    else {
        // Update the wave.
        switch(Mode->Wave.Type) {
            case WavePatternType_SpawnNForDuration: {
                auto* Pattern = &Mode->Wave.PatternSpawnNForDuration;
                Pattern->SpawnTimer += DeltaTime;
                Pattern->Timer += DeltaTime;
                if (Pattern->SpawnTimer >= Pattern->SpawnDuration ) {
                    v2f Pos = 
                        V2f_Create(Rng_Bilateral(&Mode->Rng) * Game_DesignWidth * 0.5f,
                                   Rng_Bilateral(&Mode->Rng) * Game_DesignHeight * 0.5f);
                    
                    
                    Enemy_Spawn(Mode, 
                                Assets,
                                Pos,
                                EnemyShootType_8Directions,
                                EnemyMovementType_Static);
                    
                    Pattern->SpawnTimer = 0.f;
                }
                
                if (Pattern->Timer >= Pattern->Duration) {
                    Mode->Wave.IsDone = true;
                }
                
            } break;
            default: {
                Assert(false);
            }
        }
        
    }
    
}

#endif //GAME_MODE_MAIN_WAVE_H
