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
UpdateParticlesSub(queue<particle>* Q, f32 DeltaTime, u32 Begin, u32 End) {
    for (u32 I = Begin; I <= End; ++I ) {
        particle* P = Q->Data + I;
        P->Timer += DeltaTime;
        
        P->Position += P->Direction * P->Speed * DeltaTime;
    }
}

static inline void 
UpdateParticles(game_mode_main* Mode, f32 DeltaTime) {
    queue<particle>* Q = &Mode->Particles;
    if (Queue_IsEmpty(Q)) {
        return;
    }
    // Clean up old particles
    particle * P = Queue_Next(Q);
    while(P != Null && P->Timer >= Particle_Duration) {
        Queue_Pop(Q);
        P = Queue_Next(Q);
    }
    
    // Then update the living ones
    if (Q->Begin <= Q->End) {
        UpdateParticlesSub(Q, DeltaTime, Q->Begin, Q->End);
    }
    else {
        UpdateParticlesSub(Q, DeltaTime, Q->Begin, Q->Count - 1);
        UpdateParticlesSub(Q, DeltaTime, 0, Q->End);
    }
    
}


static inline void
UpdateBulletsSub(list<bullet>* L,
                 f32 DeltaTime) 
{
    for(u32 I = 0; I < L->Count;) {
        bullet* B = Array_Get(L, I);
        
        f32 SpeedDt = B->Speed * DeltaTime;
        v2f Velocity = V2f_Mul(B->Direction, SpeedDt);
        B->Position = V2f_Add(B->Position, Velocity);
        
        if (B->Position.X <= -Game_DesignWidth * 0.5f - B->HitCircle.Radius || 
            B->Position.X >= Game_DesignWidth * 0.5f + B->HitCircle.Radius ||
            B->Position.Y <= -Game_DesignHeight * 0.5f - B->HitCircle.Radius ||
            B->Position.Y >= Game_DesignHeight * 0.5f + B->HitCircle.Radius) {
            List_Slear(L, I);
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
    for (u32 I = 0; I < Mode->Enemies.Count; ++I) 
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
            
            v2f Dir = {};
            switch (Enemy->FiringPatternType) {
                case EnemyFiringPatternType_Homing: 
                Dir = V2f_Sub(Player->Position, Enemy->Position);
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
            List_Slear(&Mode->Enemies, I);
            continue;
        }
        ++Enemy;
        
    }
}

static inline void
UpdateCollision(game_mode_main* Mode,
                f32 DeltaTime)
{
    player* Player = &Mode->Player;
    circle2f PlayerCircle = Player->HitCircle;
    v2f PlayerVel = V2f_Sub(Player->Position, Player->PrevPosition);
    PlayerCircle.Origin = V2f_Add(PlayerCircle.Origin, Player->Position);
    
    // Player vs every bullet
    for (u32 I = 0; I < Mode->DotBullets.Count;) 
    {
        bullet* DotBullet = Mode->DotBullets + I;
        circle2f DotBulletCircle = DotBullet->HitCircle;
        v2f DotBulletVel = V2f_Mul(DotBullet->Direction, DotBullet->Speed * DeltaTime);
        DotBulletCircle.Origin = V2f_Add(DotBulletCircle.Origin, DotBullet->Position);
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          DotBulletCircle,
                                          DotBulletVel)) 
            
        {
            if (Player->MoodType == MoodType_Dot) {
                List_Slear(&Mode->DotBullets, I);
                continue;
            }
        }
        ++I;
    }
    
    for (u32 I = 0; I < Mode->CircleBullets.Count;) 
    {
        bullet* B = Mode->CircleBullets + I;
        circle2f BCircle = B->HitCircle;
        v2f BVel = V2f_Mul(B->Direction, B->Speed * DeltaTime);
        BCircle.Origin = V2f_Add(BCircle.Origin, B->Position);
        
        
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          BCircle,
                                          BVel)) {
            if (Player->MoodType == MoodType_Circle) {
                List_Slear(&Mode->CircleBullets, I);
                continue;
            }
        }
        ++I;
    }
    
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
                    v2f Pos = 
                        V2f_Create(Bilateral(&Mode->Rng) * Game_DesignWidth * 0.5f,
                                   Bilateral(&Mode->Rng) * Game_DesignHeight * 0.5f);
                    auto MoodType = 
                        (enemy_mood_pattern_type)Choice(&Mode->Rng, MoodType_Count);
                    
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
    v2f Direction = {};
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
