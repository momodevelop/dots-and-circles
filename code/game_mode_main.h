#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#define MOUSE_MOVEMENT 1

// Rendering layers
// TODO: Organize this better please. 
// Maybe a map or array or something.
constexpr static f32 ZLayPlayer =  0.f;
constexpr static f32 ZLayDotBullet = 10.f;
constexpr static f32 ZLayCircleBullet = 20.f;
constexpr static f32 ZLayEnemy = 30.f;
constexpr static f32 ZLayDebug = 40.f;

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
    f32 Speed;
};


struct bullet {
    // TODO(Momo): remove this?
	atlas_aabb* ImageAabb;
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
    arena Arena;
    
    player Player;
    
    bullet* CircleBullets;
    u32 CircleBulletCount;
    u32 CircleBulletCap;
    
    bullet* DotBullets;
    u32 DotBulletCount;
    u32 DotBulletCap;
    
    enemy* Enemies;
    u32 EnemyCount;
    u32 EnemyCapacity;
    
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
    
    Assert(Mode->EnemyCount < Mode->EnemyCapacity);
    Mode->Enemies[Mode->EnemyCount++] = Enemy;
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
            Assert(Mode->DotBulletCount < Mode->DotBulletCap);
            Mode->DotBullets[Mode->DotBulletCount++] = Bullet;
        } break;
        case MoodType_Circle: {
            Assert(Mode->CircleBulletCount < Mode->CircleBulletCap);
            Mode->CircleBullets[Mode->CircleBulletCount++] = Bullet;
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
    
    Mode->Arena = Arena_SubArena(&PermState->ModeArena, Arena_Remaining(PermState->ModeArena));
    Mode->DotBulletCap = 128;
    Mode->DotBulletCount = 0;
    Mode->DotBullets = Arena_PushArray(bullet, &Mode->Arena, Mode->DotBulletCap);
    
    Mode->CircleBulletCap = 128;
    Mode->CircleBulletCount = 0;
    Mode->CircleBullets = Arena_PushArray(bullet, &Mode->Arena, Mode->CircleBulletCap);
    
    Mode->EnemyCapacity = 128;
    Mode->EnemyCount = 0;
    Mode->Enemies = Arena_PushArray(enemy, &Mode->Arena, Mode->EnemyCapacity);
    
    Mode->Wave.IsDone = true;
    Mode->Rng = Seed(0); // TODO: Used system clock for seed.
    
    assets* Assets = &TranState->Assets;
    player* Player = &Mode->Player;
    {
        Player->Speed = 300.f;
        
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
    Mode->Wave.IsDone = true;
    
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
    
#if !MOUSE_MOVEMENT
    f32 SpeedDt = Player->Speed * DeltaTime;
    v2f Velocity = V2f_Mul(Player->Direction, SpeedDt);
    Player->Position = V2f_Add(Player->Position, Velocity);
#endif
    
    
}

static inline void 
RemoveDotBullet(game_mode_main* Mode, u32 Index) {
    Mode->DotBullets[Index] = Mode->DotBullets[Mode->DotBulletCount-1];
    --Mode->DotBulletCount;
}


static inline void 
RemoveCircleBullet(game_mode_main* Mode, u32 Index) {
    Mode->CircleBullets[Index] = Mode->CircleBullets[Mode->CircleBulletCount-1];
    --Mode->CircleBulletCount;
}

static inline void
UpdateBullets(game_mode_main* Mode,
              f32 DeltaTime) 
{
    for(u32 I = 0; I < Mode->DotBulletCount;) {
        bullet* DotBullet = Mode->DotBullets + I;
        
        f32 SpeedDt = DotBullet->Speed * DeltaTime;
        v2f Velocity = V2f_Mul(DotBullet->Direction, SpeedDt);
        DotBullet->Position = V2f_Add(DotBullet->Position, Velocity);
        
        // Out of bounds self-destruction
        if (DotBullet->Position.X <= -Game_DesignWidth * 0.5f - DotBullet->HitCircle.Radius || 
            DotBullet->Position.X >= Game_DesignWidth * 0.5f + DotBullet->HitCircle.Radius ||
            DotBullet->Position.Y <= -Game_DesignHeight * 0.5f - DotBullet->HitCircle.Radius ||
            DotBullet->Position.Y >= Game_DesignHeight * 0.5f + DotBullet->HitCircle.Radius) {
            RemoveDotBullet(Mode, I);
            continue;
        }
        ++I;
    }
    
    for(u32 I = 0; I < Mode->CircleBulletCount;) {
        bullet* CircleBullet = Mode->CircleBullets + I;
        
        f32 SpeedDt = CircleBullet->Speed * DeltaTime;
        v2f Velocity = V2f_Mul(CircleBullet->Direction, SpeedDt);
        CircleBullet->Position = V2f_Add(CircleBullet->Position, Velocity);
        
        // Out of bounds self-destruction
        if (CircleBullet->Position.X <= -Game_DesignWidth * 0.5f - CircleBullet->HitCircle.Radius || 
            CircleBullet->Position.X >= Game_DesignWidth * 0.5f + CircleBullet->HitCircle.Radius ||
            CircleBullet->Position.Y <= -Game_DesignHeight * 0.5f - CircleBullet->HitCircle.Radius ||
            CircleBullet->Position.Y >= Game_DesignHeight * 0.5f + CircleBullet->HitCircle.Radius) {
            RemoveCircleBullet(Mode, I);
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
    for (u32 I = 0; I < Mode->EnemyCount; ++I) 
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
            Mode->Enemies[I] = Mode->Enemies[Mode->EnemyCount-1];
            --Mode->EnemyCount;
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
    for (u32 I = 0; I < Mode->DotBulletCount;) 
    {
        bullet* DotBullet = Mode->DotBullets + I;
        circle2f DotBulletCircle = DotBullet->HitCircle;
        DotBulletCircle.Origin = V2f_Add(DotBulletCircle.Origin, DotBullet->Position);
        
        if (Circle2f_IsIntersecting(PlayerCircle, DotBulletCircle)) {
            if (Player->MoodType == MoodType_Dot) {
                RemoveDotBullet(Mode, I);
                continue;
            }
        }
        ++I;
    }
    
    for (u32 I = 0; I < Mode->CircleBulletCount;) 
    {
        bullet* CircleBullet = Mode->CircleBullets + I;
        circle2f CircleBulletHitCircle = CircleBullet->HitCircle;
        CircleBulletHitCircle.Origin = V2f_Add(CircleBulletHitCircle.Origin, CircleBullet->Position);
        
        if (Circle2f_IsIntersecting(PlayerCircle, CircleBulletHitCircle)) {
            if (Player->MoodType == MoodType_Circle ) {
                RemoveCircleBullet(Mode, I);
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
#if !MOUSE_MOVEMENT
    
    b8 IsMovementButtonDown = false;
    if(IsDown(Input->ButtonLeft)) {
        Direction.X = -1.f;
        IsMovementButtonDown = true;
    };
    
    if(IsDown(Input->ButtonRight)) {
        Direction.X = 1.f;
        IsMovementButtonDown = true;
    }
    
    if(IsDown(Input->ButtonUp)) {
        Direction.Y = 1.f;
        IsMovementButtonDown = true;
    }
    if(IsDown(Input->ButtonDown)) {
        Direction.Y = -1.f;
        IsMovementButtonDown = true;
    }
    
    if (IsMovementButtonDown) 
        Player->Direction = V2f_Normalize(Direction);
    else {
        Player->Direction = {};
    }
#else 
    // TODO(Momo): Hacky way to convert screen space to world space
    // It should really be based on camera position and all that...
    // I guess we should have a camera class to manage all this if we 
    // really want
    Player->Position.X = Input->DesignMousePos.X - Game_DesignWidth * 0.5f;
    Player->Position.Y = -(Input->DesignMousePos.Y - Game_DesignHeight * 0.5f);
    
    
#endif
    
    
    // NOTE(Momo): Absorb Mode Switch
    if(IsPoked(Input->ButtonSwitch)) {
        Player->MoodType = 
            (Player->MoodType == MoodType_Dot) ? MoodType_Circle : MoodType_Dot;
        
        switch(Player->MoodType) {
            case MoodType_Dot: {
                Player->DotImageAlphaTarget = 1.f;
            } break;
            case MoodType_Circle: {
                Player->DotImageAlphaTarget = 0.f;
            }break;
            default:
            Assert(false);
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
        for (u32 I = 0; I < Mode->DotBulletCount; ++I) {
            bullet* DotBullet = Mode->DotBullets + I;
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
        for (u32 I = 0; I < Mode->CircleBulletCount; ++I) {
            bullet* CircleBullet = Mode->CircleBullets + I;
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
    for(u32 I = 0; I < Mode->EnemyCount; ++I )
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
    SwitchToGameCoords(RenderCommands);
    game_mode_main* Mode = PermState->MainMode;
    Renderer_ClearColor(RenderCommands, C4f_Create(0.15f, 0.15f, 0.15f, 1.f));
    
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
    
    
    DebugInspector_PushU32(&DebugState->Inspector, 
                           U8CStr_FromSiStr("Dots: "), 
                           Mode->DotBulletCount);
    DebugInspector_PushU32(&DebugState->Inspector, 
                           U8CStr_FromSiStr("Circles: "), 
                           Mode->CircleBulletCount);
    
    DebugInspector_PushU32(&DebugState->Inspector, 
                           U8CStr_FromSiStr("Bullets: "), 
                           Mode->DotBulletCount + Mode->CircleBulletCount);
    
}

#endif //GAME_MODE_H
