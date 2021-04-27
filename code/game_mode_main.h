#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H


#define ZLayPlayer 0.f
#define ZLayDotBullet 10.f
#define ZLayCircleBullet 20.f
#define ZLayEnemy 30.f
#define ZLayDebug 40.f
#define CircleCap 128
#define DotCap 128
#define EnemyCap 128

enum mood_type {
    MoodType_Dot,
    MoodType_Circle,
    
    MoodType_Count,
};

enum enemy_mood_pattern_type {
    EnemyMoodPatternType_Dot,
    EnemyMoodPatternType_Circle,
    EnemyMoodPatternType_Both,
    
    EnemyMoodPatternType_Count,
};

enum enemy_firing_pattern_type  {
    EnemyFiringPatternType_Homing,
    
    EnemyFiringPatternType_Count,
};

enum enemy_movement_type {
    EnemyMovementType_Static,
    
    EnemyMovementType_Count,
};


// Wave
enum wave_pattern_type {
    WavePatternType_SpawnNForDuration,   // Spawns N enemies at a time
};

struct wave_pattern_spawn_n_for_duration {
    u32 EnemiesPerSpawn;
    f32 SpawnTimer;
    f32 SpawnDuration;
    f32 Timer;
    f32 Duration;
};

struct wave {
    wave_pattern_type Type;
    union {
        wave_pattern_spawn_n_for_duration PatternSpawnNForDuration;    
    };
    b32 IsDone;
};

struct player {
    // NOTE(Momo): Rendering
    f32 DotImageAlpha;
    f32 DotImageAlphaTarget;
    f32 DotImageTransitionTimer;
    f32 DotImageTransitionDuration;
    
    v2f Size;
    
	// Collision
    circle2f HitCircle;
    
    
    // Physics
    v2f Position;
    v2f Direction; 
    
    // Gameplay
    mood_type MoodType;
};


struct bullet {
    v2f Size;
    mood_type MoodType;
    v2f Direction;
	v2f Position;
	f32 Speed;
	circle2f HitCircle; 
};

struct enemy {
	v2f Size; 
	v2f Position;
    
    enemy_firing_pattern_type FiringPatternType;
    enemy_mood_pattern_type MoodPatternType;
    enemy_movement_type MovementType;
    
    f32 FireTimer;
	f32 FireDuration;
    
    f32 LifeTimer;
    f32 LifeDuration;
};


struct game_mode_main {
    arena_mark ArenaMark;
    
    player Player;
    game_camera Camera;
    
    
    list<bullet> CircleBullets;
    list<bullet> DotBullets;
    list<enemy> Enemies;
    
    wave Wave;
    rng_series Rng;
};


static inline void
SpawnEnemy(game_mode_main* Mode, 
           assets* Assets, 
           v2f Position,
           enemy_mood_pattern_type MoodPatternType,
           enemy_firing_pattern_type FiringPatternType, 
           enemy_movement_type MovementType, 
           f32 FireRate,
           f32 LifeDuration) 
{
    enemy Enemy = {}; 
    Enemy.Position = Position;
    Enemy.Size = V2f_Create(32.f, 32.f);
    
    Enemy.FireTimer = 0.f;
    Enemy.FireDuration = FireRate; 
    Enemy.LifeDuration = LifeDuration;
    
    Enemy.FiringPatternType = FiringPatternType;
    Enemy.MoodPatternType = MoodPatternType;
    Enemy.MovementType = MovementType;
    
    List_PushItem(&Mode->Enemies, Enemy);
}


static inline void
SpawnBullet(game_mode_main* Mode, assets* Assets, v2f Position, v2f Direction, f32 Speed, mood_type Mood) {
    bullet Bullet = {}; 
    Bullet.Position = Position;
	Bullet.Speed = Speed;
    Bullet.Size = V2f_Create(16.f, 16.f);
    
    Bullet.HitCircle = {
        V2f_Create(0.f, 0.f), 
        Bullet.Size.X * 0.5f 
    };
    
    if (V2f_LengthSq(Direction) > 0.f) {
	    Bullet.Direction = V2f_Normalize(Direction);
    }
    switch (Mood) {
        case MoodType_Dot: {
            List_PushItem(&Mode->DotBullets, Bullet);
        } break;
        case MoodType_Circle: {
            List_PushItem(&Mode->CircleBullets, Bullet);
        } break;
        default: {
            Assert(False);
        }
    }
    
}

static inline void 
InitMainMode(permanent_state* PermState,
             transient_state* TranState,
             debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    // NOTE(Momo): Init camera
    {
        Mode->Camera.Position = V3f_Create(0.f, 0.f, 0.f);
        Mode->Camera.Anchor = V3f_Create(0.5f, 0.5f, 0.5f);
        Mode->Camera.Color = Color_Grey2;
        Mode->Camera.Dimensions = V3f_Create(Game_DesignWidth,
                                             Game_DesignHeight,
                                             Game_DesignDepth);
    }
    
    Mode->ArenaMark = Arena_Mark(&PermState->ModeArena);
    List_InitFromArena(&Mode->DotBullets, Mode->ArenaMark, DotCap);
    List_InitFromArena(&Mode->CircleBullets, Mode->ArenaMark, CircleCap);
    List_InitFromArena(&Mode->Enemies, Mode->ArenaMark, EnemyCap);
    
    Mode->Wave.IsDone = True;
    Mode->Rng = Seed(0); // TODO: Used system clock for seed.
    
    assets* Assets = &TranState->Assets;
    player* Player = &Mode->Player;
    {
        Player->Position = {};
        Player->Direction = {};
        Player->Size = V2f_Create( 32.f, 32.f );
        Player->HitCircle = { v2f{}, 16.f};
        
        // NOTE(Momo): We start as Dot
        Player->MoodType = MoodType_Dot;
        Player->DotImageAlpha = 1.f;
        Player->DotImageAlphaTarget = 1.f;
        
        Player->DotImageTransitionDuration = 0.1f;
        Player->DotImageTransitionTimer = Player->DotImageTransitionDuration;
    }
    Mode->Wave.IsDone = True;
    
}


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
UpdateBullets(game_mode_main* Mode,
              f32 DeltaTime) 
{
    for(u32 I = 0; I < Mode->DotBullets.Count;) {
        bullet* DotBullet = Mode->DotBullets.Data + I;
        
        f32 SpeedDt = DotBullet->Speed * DeltaTime;
        v2f Velocity = V2f_Mul(DotBullet->Direction, SpeedDt);
        DotBullet->Position = V2f_Add(DotBullet->Position, Velocity);
        
        // Out of bounds self-destruction
        if (DotBullet->Position.X <= -Game_DesignWidth * 0.5f - DotBullet->HitCircle.Radius || 
            DotBullet->Position.X >= Game_DesignWidth * 0.5f + DotBullet->HitCircle.Radius ||
            DotBullet->Position.Y <= -Game_DesignHeight * 0.5f - DotBullet->HitCircle.Radius ||
            DotBullet->Position.Y >= Game_DesignHeight * 0.5f + DotBullet->HitCircle.Radius) {
            List_Slear(&Mode->DotBullets, I);
            continue;
        }
        ++I;
    }
    
    for(u32 I = 0; I < Mode->CircleBullets.Count;) {
        bullet* CircleBullet = Mode->CircleBullets.Data + I;
        
        f32 SpeedDt = CircleBullet->Speed * DeltaTime;
        v2f Velocity = V2f_Mul(CircleBullet->Direction, SpeedDt);
        CircleBullet->Position = V2f_Add(CircleBullet->Position, Velocity);
        
        // Out of bounds self-destruction
        if (CircleBullet->Position.X <= -Game_DesignWidth * 0.5f - CircleBullet->HitCircle.Radius || 
            CircleBullet->Position.X >= Game_DesignWidth * 0.5f + CircleBullet->HitCircle.Radius ||
            CircleBullet->Position.Y <= -Game_DesignHeight * 0.5f - CircleBullet->HitCircle.Radius ||
            CircleBullet->Position.Y >= Game_DesignHeight * 0.5f + CircleBullet->HitCircle.Radius) {
            List_Slear(&Mode->CircleBullets, I);
            continue;
        }
        ++I;
    }
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
UpdateCollision(game_mode_main* Mode)
{
    player* Player = &Mode->Player;
    circle2f PlayerCircle = Player->HitCircle;
    PlayerCircle.Origin = V2f_Add(PlayerCircle.Origin, Player->Position);
    
    // Player vs every bullet
    for (u32 I = 0; I < Mode->DotBullets.Count;) 
    {
        bullet* DotBullet = Mode->DotBullets + I;
        circle2f DotBulletCircle = DotBullet->HitCircle;
        DotBulletCircle.Origin = V2f_Add(DotBulletCircle.Origin, DotBullet->Position);
        
        if (Bonk2_IsCircleXCircle(PlayerCircle, DotBulletCircle)) {
            if (Player->MoodType == MoodType_Dot) {
                List_Slear(&Mode->DotBullets, I);
                continue;
            }
        }
        ++I;
    }
    
    for (u32 I = 0; I < Mode->CircleBullets.Count;) 
    {
        bullet* CircleBullet = Mode->CircleBullets.Data + I;
        circle2f CircleBulletHitCircle = CircleBullet->HitCircle;
        CircleBulletHitCircle.Origin = V2f_Add(CircleBulletHitCircle.Origin, CircleBullet->Position);
        
        if (Bonk2_IsCircleXCircle(PlayerCircle, CircleBulletHitCircle)) {
            if (Player->MoodType == MoodType_Circle ) {
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
            default:
            Assert(false);
        }
        
    }
    
}

static inline void
UpdateInput(game_mode_main* Mode,
            game_input* Input)
{
    v2f Direction = {};
    player* Player = &Mode->Player; 
    
    // TODO(Momo): Hacky way to convert screen space to world space
    // It should really be based on camera position and all that...
    // I guess we should have a camera class to manage all this if we 
    // really want
    Player->Position = Hack_ScreenToWorldSpace(Input->DesignMousePos);
    
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


static inline void 
RenderPlayer(game_mode_main* Mode,
             assets* Assets,
             mailbox* RenderCommands) 
{
    player* Player = &Mode->Player;
    m44f S = M44f_Scale(Player->Size.X, Player->Size.Y, 1.f);
    
    {
        m44f T = M44f_Translation(Player->Position.X,
                                  Player->Position.Y,
                                  ZLayPlayer);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, 1.f - Player->DotImageAlpha);
        
        Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                       Assets,
                                       AtlasAabb_PlayerCircle,
                                       M44f_Concat(T,S), 
                                       Color);
    }
    
    {
        m44f T = M44f_Translation(Player->Position.X,
                                  Player->Position.Y,
                                  ZLayPlayer + 0.01f);
        c4f Color = C4f_Create(1.f, 1.f, 1.f, Player->DotImageAlpha);
        Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                       Assets,
                                       AtlasAabb_PlayerDot,
                                       M44f_Concat(T,S), 
                                       Color);
    }
}

static inline void
RenderBullets(game_mode_main* Mode,
              assets* Assets,
              mailbox* RenderCommands) 
{
    // Bullet Rendering.
    // NOTE(Momo): Circles are in front of Dots and are therefore 'nearer'.
    // Thus we have to render Dots first before Circles.
    
    // Render Dots
    {
        f32 LayerOffset = 0.f;
        atlas_aabb* AtlasAabb = Assets->AtlasAabbs + AtlasAabb_BulletDot;
        texture* Texture = Assets->Textures + AtlasAabb->TextureId;
        for (u32 I = 0; I < Mode->DotBullets.Count; ++I) {
            bullet* DotBullet = Mode->DotBullets.Data + I;
            m44f S = M44f_Scale(DotBullet->Size.X, 
                                DotBullet->Size.Y, 
                                1.f);
            
            m44f T = M44f_Translation(DotBullet->Position.X,
                                      DotBullet->Position.Y,
                                      ZLayDotBullet + LayerOffset);
            
            Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                           Assets,
                                           AtlasAabb_BulletDot,
                                           M44f_Concat(T,S), 
                                           Color_White);
            LayerOffset += 0.01f;
            
        }
        
    }
    
    // Render Circles
    {
        f32 LayerOffset = 0.f;
        for (u32 I = 0; I < Mode->CircleBullets.Count; ++I) {
            bullet* CircleBullet = Mode->CircleBullets.Data + I;
            m44f S = M44f_Scale(CircleBullet->Size.X, 
                                CircleBullet->Size.Y, 
                                1.f);
            
            m44f T = M44f_Translation(CircleBullet->Position.X,
                                      CircleBullet->Position.Y,
                                      ZLayCircleBullet + LayerOffset);
            
            Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                           Assets,
                                           AtlasAabb_BulletCircle,
                                           M44f_Concat(T,S), 
                                           Color_White);
            LayerOffset += 0.01f;
            
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
        m44f S = M44f_Scale(Enemy->Size.X, Enemy->Size.Y, 1.f);
        m44f T = M44f_Translation(Enemy->Position.X,
                                  Enemy->Position.Y,
                                  ZLayEnemy);
        
        Draw_TexturedQuadFromAtlasAabb(RenderCommands,
                                       Assets,
                                       AtlasAabb_Enemy,
                                       M44f_Concat(T,S), 
                                       Color_White);
    }
}


static inline void
UpdateMainMode(permanent_state* PermState, 
               transient_state* TranState,
               debug_state* DebugState,
               mailbox* RenderCommands, 
               game_input* Input,
               f32 DeltaTime) 
{
    game_mode_main* Mode = PermState->MainMode;
    Camera_Set(&Mode->Camera, RenderCommands);
    
    assets* Assets = &TranState->Assets;
    UpdateInput(Mode, Input);
    UpdatePlayer(Mode, DeltaTime);    
    UpdateBullets(Mode, DeltaTime);
    UpdateWaves(Mode, Assets, DeltaTime);
    UpdateEnemies(Mode, Assets, DeltaTime); 
    UpdateCollision(Mode);
    
    
    
    RenderPlayer(Mode, Assets, RenderCommands);
    RenderBullets(Mode, Assets, RenderCommands);
    RenderEnemies(Mode, Assets, RenderCommands);
    
    u8_cstr Buffer = {};
    U8CStr_InitFromSiStr(&Buffer, "Dots: ");
    DebugInspector_PushU32(&DebugState->Inspector,
                           Buffer,
                           Mode->DotBullets.Count);
    U8CStr_InitFromSiStr(&Buffer, "Circles: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->CircleBullets.Count);
    
    U8CStr_InitFromSiStr(&Buffer, "Bullets: ");
    DebugInspector_PushU32(&DebugState->Inspector, 
                           Buffer, 
                           Mode->DotBullets.Count + Mode->CircleBullets.Count);
    
}

#endif //GAME_MODE_H
