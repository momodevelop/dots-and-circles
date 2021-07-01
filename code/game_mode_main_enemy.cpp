

static inline void
Enemy_SetStateSpawn(enemy* E) {
    E->State = EnemyState_Spawning;
    E->StateSpawn.Timer = 0.f;
}

static inline void
Enemy_SetStateDying(enemy* E) {
    E->State = EnemyState_Dying;
    E->StateDying.Timer = 0.f;
}

static inline void
Enemy_SetStateActive(enemy* E) {
    E->State = EnemyState_Active;
    E->StateActive.Timer = 0.f;
}

static inline void
Enemy_Spawn(game_mode_main* Mode,
            v2f Position,
            enemy_shoot_type ShootType, 
            enemy_movement_type MovementType) 
{
    enemy* Enemy = Mode->Enemies.push();
    Enemy->Position = Position;
    
    // NOTE(Momo): Shoot
    Enemy->ShootType = ShootType;
    switch(Enemy->ShootType) {
        case EnemyShootType_Homing: {
            enemy_shoot_homing* Shoot = &Enemy->ShootHoming;
            Shoot->Timer = 0.f;
            Shoot->Duration = 0.1f;
            Shoot->Mood = (mood_type)Mode->rng.choice(MoodType_Count);
        } break;
        case EnemyShootType_8Directions: {
            enemy_shoot_8dir* Shoot = &Enemy->Shoot8Dir;
            Shoot->Timer = 0.f;
            Shoot->Duration = 0.1f;
            Shoot->Mood = (mood_type)Mode->rng.choice(MoodType_Count);
        } break;
    };
    
    Enemy->MovementType = MovementType;
    
    Enemy_SetStateSpawn(Enemy);
}

static inline void
Enemy_DoStateActive(enemy* Enemy, player* Player, game_mode_main* Mode, f32 DeltaTime) {
    enemy_state_active* Active = &Enemy->StateActive;
    // Movement
    switch( Enemy->MovementType ) {
        case EnemyMovementType_Static:
        // Do nothing
        break;
        default: {
            ASSERT(false);
        }
    }
    
    // Rotation
    Enemy->Rotation += Enemy->RotationSpeed * DeltaTime;
    Enemy->RotationSpeed += (Enemy->ActiveStateRotationSpeed - Enemy->RotationSpeed) * 0.9f;
    
    // Fire
    switch (Enemy->ShootType) {
        case EnemyShootType_Homing: {
            enemy_shoot_homing* Shoot = &Enemy->ShootHoming;
            Shoot->Timer += DeltaTime;
            if (Shoot->Timer > Shoot->Duration) {
                v2f Direction = normalize(Player->Position - Enemy->Position);
                Bullet_Spawn(Mode, Enemy->Position, Direction, 200.f, Shoot->Mood);
                Shoot->Timer = 0.f;
            }
        } break;
        case EnemyShootType_8Directions: {
            enemy_shoot_8dir* Shoot = &Enemy->Shoot8Dir;
            Shoot->Timer += DeltaTime;
            static m22f RotateMtx = m22f::create_rotation(TAU/8);
            if (Shoot->Timer > Shoot->Duration) {
                v2f Dir = v2f::create(1.f, 0.f);
                for (u32 I = 0; I < 8; ++I) {
                    Dir = RotateMtx * Dir;
                    Bullet_Spawn(Mode, Enemy->Position, Dir, 200.f, Shoot->Mood);
                }
                Shoot->Timer = 0.f;
            }
            
        } break;
        default: {
            ASSERT(false);
        }
    }
    
    // Life time
    Active->Timer += DeltaTime;
    if (Active->Timer > Active->Duration) {
        Enemy_SetStateDying(Enemy);
    }
}


static inline void 
Main_UpdateEnemies(game_mode_main* Mode,
                   f32 DeltaTime) 
{
    player* Player = &Mode->Player;
    auto SlearIfLamb = [&](enemy* Enemy) {
        switch(Enemy->State) {
            case EnemyState_Spawning: {
                enemy_state_spawn* Spawn = &Enemy->StateSpawn;
                Spawn->Timer += DeltaTime;
                
                Enemy->Rotation += DeltaTime * Enemy->SpawnStateRotationSpeed;
                if (Spawn->Timer >= Spawn->Duration) {
                    Enemy_SetStateActive(Enemy);
                }
            } break;
            case EnemyState_Active: {
                Enemy_DoStateActive(Enemy, Player, Mode, DeltaTime);
            } break;
            case EnemyState_Dying: {
                enemy_state_dying* Dying = &Enemy->StateDying;
                Dying->Timer += DeltaTime;
                Enemy->Rotation += DeltaTime * Enemy->DieStateRotationSpeed;
                
                return Dying->Timer >= Dying->Duration;
            } break;
        }
        return false;
    };
    Mode->Enemies.foreach_slear_if(SlearIfLamb);
    
}

static inline void
Main_RenderEnemies(game_mode_main* Mode) 
{
    u32 CurrentCount = 0;
    auto ForEachLamb = [&](enemy* Enemy){
        f32 Offset = 0.00001f * CurrentCount++;
        f32 Size = Enemy->Size;
        switch(Enemy->State) {
            case EnemyState_Spawning: {
                enemy_state_spawn* Spawn = &Enemy->StateSpawn;
                f32 Ease = ease_out_quad(Spawn->Timer/Spawn->Duration);
                Size = Enemy->Size * Ease;
            } break;
            case EnemyState_Dying: {
                enemy_state_dying* Dying = &Enemy->StateDying;
                f32 Ease = 1.f - Dying->Timer/Dying->Duration;
                Size = Enemy->Size * Ease;
            }
        }
        m44f S = m44f::create_scale(Size, Size, 1.f);
        m44f R = m44f::create_rotation_z(Enemy->Rotation);
        m44f T = m44f::create_translation(Enemy->Position.x,
                                          Enemy->Position.y,
                                          ZLayEnemy + Offset);
        
        Draw_TexturedQuadFromImage(IMAGE_ENEMY,
                                   T*R*S, 
                                   C4F_WHITE);
    };
    Mode->Enemies.foreach(ForEachLamb);
}
