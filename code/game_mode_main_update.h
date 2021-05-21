/* date = May 9th 2021 5:43 pm */

#ifndef GAME_MODE_MAIN_UPDATE_H
#define GAME_MODE_MAIN_UPDATE_H


static inline void 
UpdatePlayer(game_mode_main* Mode, 
             f32 DeltaTime) 
{
    player* Player = &Mode->Player; 
    Player->DotImageAlpha = Lerp(1.f - Player->DotImageAlphaTarget, 
                                 Player->DotImageAlphaTarget, 
                                 Player->DotImageTransitionTimer / Player->DotImageTransitionDuration);
    
    Player->DotImageTransitionTimer += DeltaTime;
    Player->DotImageTransitionTimer = 
        Clamp(Player->DotImageTransitionTimer, 
              0.f, 
              Player->DotImageTransitionDuration);
    
}

static inline void
UpdateParticlesSub(MM_Queue<particle>* Q, f32 DeltaTime, u32 Begin, u32 End) {
    for (u32 I = Begin; I <= End; ++I ) {
        particle* P = MM_Queue_Get(Q, I);
        P->Timer += DeltaTime;
        
        MM_V2f Velocity = MM_V2f_Mul(P->Direction, P->Speed * DeltaTime);
        
        P->Position = MM_V2f_Add(P->Position, Velocity);
    }
}

static inline void 
UpdateParticles(game_mode_main* Mode, f32 DeltaTime) {
    MM_Queue<particle>* Q = &Mode->Particles;
    if (MM_Queue_IsEmpty(Q)) {
        return;
    }
    // Clean up old particles
    particle * P = MM_Queue_Next(Q);
    while(P != Null && P->Timer >= Particle_Duration) {
        MM_Queue_Pop(Q);
        P = MM_Queue_Next(Q);
    }
    
    // Then update the living ones
    if (Q->begin <= Q->end) {
        UpdateParticlesSub(Q, DeltaTime, Q->begin, Q->end);
    }
    else {
        UpdateParticlesSub(Q, DeltaTime, Q->begin, Q->count - 1);
        UpdateParticlesSub(Q, DeltaTime, 0, Q->end);
    }
    
}


static inline void
UpdateBulletsSub(MM_List<bullet>* L,
                 f32 DeltaTime) 
{
    for(u32 I = 0; I < L->count;) {
        bullet* B = MM_List_Get(L, I);
        
        f32 SpeedDt = B->Speed * DeltaTime;
        MM_V2f Velocity = MM_V2f_Mul(B->Direction, SpeedDt);
        B->Position = MM_V2f_Add(B->Position, Velocity);
        
        if (B->Position.x <= -Game_DesignWidth * 0.5f - B->HitCircle.radius || 
            B->Position.x >= Game_DesignWidth * 0.5f + B->HitCircle.radius ||
            B->Position.y <= -Game_DesignHeight * 0.5f - B->HitCircle.radius ||
            B->Position.y >= Game_DesignHeight * 0.5f + B->HitCircle.radius) {
            MM_List_Slear(L, I);
            continue;
        }
        ++I;
    }
    
}

static inline void
UpdateBullets(game_mode_main* Mode,
              f32 DeltaTime) 
{
    UpdateBulletsSub(&Mode->DotBullets, DeltaTime);
    UpdateBulletsSub(&Mode->CircleBullets, DeltaTime);
}

static inline void 
UpdateEnemies(game_mode_main* Mode,
              assets* Assets,
              f32 DeltaTime) 
{
    player* Player = &Mode->Player;
    for (u32 I = 0; I < Mode->Enemies.count; ++I) 
    {
        enemy* Enemy = Mode->Enemies + I;
        
        // Movement
        switch( Enemy->MovementType ) {
            case EnemyMovementType_Static:
            // Do nothing
            break;
            default: 
            Assert(false);
        }
        
        // Fire
        Enemy->FireTimer += DeltaTime;
        if (Enemy->FireTimer > Enemy->FireDuration) {       
            mood_type MoodType = {};
            switch (Enemy->MoodPatternType) {
                case EnemyMoodPatternType_Dot: 
                MoodType = MoodType_Dot;
                break;
                case EnemyMoodPatternType_Circle:
                MoodType = MoodType_Circle;
                break;
                default:
                Assert(false);
            }
            
            MM_V2f Dir = {};
            switch (Enemy->FiringPatternType) {
                case EnemyFiringPatternType_Homing: 
                Dir = MM_V2f_Sub(Player->Position, Enemy->Position);
                break;
                default:
                Assert(false);
            }
            SpawnBullet(Mode, Assets, Enemy->Position, Dir, 200.f, MoodType);
            Enemy->FireTimer = 0.f;
        }
        
        // Life time
        Enemy->LifeTimer += DeltaTime;
        if (Enemy->LifeTimer > Enemy->LifeDuration) {
            // Quick removal
            MM_List_Slear(&Mode->Enemies, I);
            continue;
        }
        ++Enemy;
        
    }
}

static inline void
UpdateCollisionSub(game_mode_main* Mode,
                   player* Player,
                   MM_List<bullet>* Bullets,
                   assets* Assets,
                   f32 DeltaTime) 
{
    MM_Circle2f PlayerCircle = Player->HitCircle;
    MM_V2f PlayerVel = MM_V2f_Sub(Player->Position, Player->PrevPosition);
    PlayerCircle.origin = MM_V2f_Add(PlayerCircle.origin, Player->Position);
    
    // Player vs every bullet
    for (u32 I = 0; I < Bullets->count;) 
    {
        bullet* B = MM_List_Get(Bullets, I);
        MM_Circle2f BCircle = B->HitCircle;
        MM_V2f BVel = MM_V2f_Mul(B->Direction, B->Speed * DeltaTime);
        BCircle.origin = MM_V2f_Add(BCircle.origin, B->Position);
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          BCircle,
                                          BVel)) 
            
        {
            if (Player->MoodType == B->MoodType) {
#if 0
                SpawnParticleRandomDirectionAndSpeed(Mode,
                                                     Assets,
                                                     {});
#endif
                
                MM_List_Slear(Bullets, I);
                continue;
            }
        }
        ++I;
    }
    
}


static inline void
UpdateCollision(game_mode_main* Mode,
                assets* Assets,
                f32 DeltaTime)
{
    UpdateCollisionSub(Mode, &Mode->Player, &Mode->DotBullets, Assets, DeltaTime);
    UpdateCollisionSub(Mode, &Mode->Player, &Mode->CircleBullets, Assets, DeltaTime);
    
}

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
            default: 
            Assert(false);
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
                    MM_V2f Pos = 
                        MM_V2f_Create(MM_Rng_Bilateral(&Mode->Rng) * Game_DesignWidth * 0.5f,
                                   MM_Rng_Bilateral(&Mode->Rng) * Game_DesignHeight * 0.5f);
                    auto MoodType = (enemy_mood_pattern_type)MM_Rng_Choice(&Mode->Rng, MoodType_Count);
                    
                    SpawnEnemy(Mode, 
                               Assets,
                               Pos,
                               MoodType,
                               EnemyFiringPatternType_Homing,
                               EnemyMovementType_Static,
                               0.1f, 
                               10.f);
                    
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

static inline void
UpdateInput(game_mode_main* Mode,
            platform_input* Input)
{
    MM_V2f Direction = {};
    player* Player = &Mode->Player; 
    
    Player->PrevPosition = Player->Position;
    Player->Position = Camera_ScreenToWorld(&Mode->Camera,
                                            Input->DesignMousePos);
    
    
    // NOTE(Momo): Absorb Mode Switch
    if(Button_IsPoked(Input->ButtonSwitch)) {
        Player->MoodType = 
            (Player->MoodType == MoodType_Dot) ? MoodType_Circle : MoodType_Dot;
        
        switch(Player->MoodType) {
            case MoodType_Dot: {
                Player->DotImageAlphaTarget = 1.f;
            } break;
            case MoodType_Circle: {
                Player->DotImageAlphaTarget = 0.f;
            }break;
            default:{ 
                Assert(false);
            }
        }
        Player->DotImageTransitionTimer = 0.f;
    }
}

#endif //GAME_MODE_MAIN_UPDATE_H
