/* date = May 22nd 2021 7:08 pm */

#ifndef GAME_MODE_MAIN_ENEMY_H
#define GAME_MODE_MAIN_ENEMY_H

static inline void
SpawnEnemy(game_mode_main* Mode, 
           assets* Assets, 
           v2f Position,
           enemy_mood_pattern_type MoodPatternType,
           enemy_firing_pattern_type FiringPatternType, 
           enemy_movement_type MovementType) 
{
    enemy* Enemy = List_Push(&Mode->Enemies);
    Enemy->Position = Position;
    
    Enemy->FireTimer = 0.f;
    Enemy->LifeTimer = 0.f;
    
    Enemy->FiringPatternType = FiringPatternType;
    Enemy->MoodPatternType = MoodPatternType;
    Enemy->MovementType = MovementType;
    Enemy->SpawnTimer = 0.f;
    Enemy->DieTimer = 0.f;
    Enemy->State = EnemyState_Spawning;
    
}


static inline void
UpdateEnemyActiveBehaviour(enemy* Enemy, player* Player, game_mode_main* Mode, assets* Assets, f32 DeltaTime) {
    // Movement
    switch( Enemy->MovementType ) {
        case EnemyMovementType_Static:
        // Do nothing
        break;
        default: {
            Assert(false);
        }
    }
    
    // Rotation
    Enemy->Rotation += Enemy->RotationSpeed * DeltaTime;
    Enemy->RotationSpeed += (Enemy->ActiveStateRotationSpeed - Enemy->RotationSpeed) * 0.9f;
    
    // Fire
    Enemy->FireTimer += DeltaTime;
    if (Enemy->FireTimer > Enemy->FireDuration) {       
        mood_type MoodType = {};
        switch (Enemy->MoodPatternType) {
            case EnemyMoodPatternType_Dot: {
                MoodType = MoodType_Dot;
            } break;
            case EnemyMoodPatternType_Circle: {
                MoodType = MoodType_Circle;
            } break;
            default: {
                Assert(false);
            }
        }
        
        v2f Dir = {};
        switch (Enemy->FiringPatternType) {
            case EnemyFiringPatternType_Homing: {
                Dir = Player->Position - Enemy->Position;
            } break;
            default: {
                Assert(false);
            }
        }
        SpawnBullet(Mode, Assets, Enemy->Position, Dir, 200.f, MoodType);
        Enemy->FireTimer = 0.f;
    }
    
    // Life time
    Enemy->LifeTimer += DeltaTime;
    if (Enemy->LifeTimer > Enemy->LifeDuration) {
        Enemy->State = EnemyState_Dying;
    }
}

static inline void 
UpdateEnemies(game_mode_main* Mode,
              assets* Assets,
              f32 DeltaTime) 
{
    player* Player = &Mode->Player;
    for (u32 I = 0; I < Mode->Enemies.Count;) 
    {
        enemy* Enemy = Mode->Enemies + I;
        
        switch(Enemy->State) {
            case EnemyState_Spawning: {
                Enemy->SpawnTimer += DeltaTime;
                Enemy->Rotation += DeltaTime * Enemy->SpawnStateRotationSpeed;
                if (Enemy->SpawnTimer >= Enemy->SpawnDuration) {
                    Enemy->State = EnemyState_Active;
                    Enemy->SpawnTimer = Enemy->SpawnDuration;
                }
                ++I;
            } break;
            case EnemyState_Active: {
                UpdateEnemyActiveBehaviour(Enemy, Player, Mode, Assets, DeltaTime);
                ++I;
            } break;
            case EnemyState_Dying: {
                Enemy->DieTimer += DeltaTime;
                Enemy->Rotation += DeltaTime * Enemy->DieStateRotationSpeed;
                if (Enemy->DieTimer >= Enemy->DieDuration) {
                    Enemy->DieTimer = Enemy->DieDuration;
                    List_Slear(&Mode->Enemies, I);
                    continue;
                }
                ++I;
            } break;
        }
    }
    
    
}

static inline void
RenderEnemies(game_mode_main* Mode, 
              assets* Assets,
              mailbox* RenderCommands) 
{
    for(u32 I = 0; I < Mode->Enemies.Count; ++I )
    {
        enemy* Enemy = Mode->Enemies + I;
        f32 Offset = 0.001f * I;
        
        f32 Size = Enemy->Size;
        switch(Enemy->State) {
            case EnemyState_Spawning: {
                f32 Ease = EaseOutQuad(Enemy->SpawnTimer/Enemy->SpawnDuration);
                Size = Enemy->Size * Ease;
            } break;
            case EnemyState_Dying: {
                f32 Ease = 1.f - Enemy->DieTimer/Enemy->DieDuration;
                Size = Enemy->Size * Ease;
            }
        }
        m44f S = M44f_Scale(Size, Size, 1.f);
        m44f R = M44f_RotationZ(Enemy->Rotation);
        m44f T = M44f_Translation(Enemy->Position.X,
                                  Enemy->Position.Y,
                                  ZLayEnemy + Offset);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_Enemy,
                                   T*R*S, 
                                   Color_White);
    }
}

#endif //GAME_MODE_MAIN_ENEMY_H
