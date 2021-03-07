#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "mm_easing.h"
#include "mm_maths.h"
#include "mm_random.h"
#include "mm_list.h"
#include "game.h"

// Rendering layers
// TODO: Organize this better please. 
// Maybe a map or array or something.
constexpr static f32 ZLayPlayer =  0.f;
constexpr static f32 ZLayDotBullet = 10.f;
constexpr static f32 ZLayCircleBullet = 20.f;
constexpr static f32 ZLayEnemy = 30.f;
constexpr static f32 ZLayDebug = 40.f;

enum mood_type : u32 {
    MoodType_Dot,
    MoodType_Circle,
    
    MoodType_Count,
};

enum enemy_mood_pattern_type : u32 {
    EnemyMoodPatternType_Dot,
    EnemyMoodPatternType_Circle,
    EnemyMoodPatternType_Both,
    
    EnemyMoodPatternType_Count,
};

enum enemy_firing_pattern_type : u32 {
    EnemyFiringPatternType_Homing,
    
    EnemyFiringPatternType_Count,
};

enum enemy_movement_type : u32 {
    EnemyMovementType_Static,
    
    EnemyMovementType_Count,
};


// Wave
enum wave_pattern_type : u32 {
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
    game_asset_atlas_aabb* DotImageAabb;
    game_asset_atlas_aabb* CircleImageAabb;
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
	game_asset_atlas_aabb* ImageAabb;
	v2f Size;
    mood_type MoodType;
    v2f Direction;
	v2f Position;
	f32 Speed;
	circle2f HitCircle; 
};

struct enemy {
    game_asset_atlas_aabb* ImageAabb;
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
    
    list<bullet> Bullets;
    list<enemy> Enemies;
    
    wave Wave;
    rng_series Rng;
};


static inline void
SpawnEnemy(game_mode_main* Mode, 
           game_assets* Assets, 
           v2f Position,
           enemy_mood_pattern_type MoodPatternType,
           enemy_firing_pattern_type FiringPatternType, 
           enemy_movement_type MovementType, 
           f32 FireRate,
           f32 LifeDuration) 
{
    enemy Enemy = {}; 
    Enemy.Position = Position;
    Enemy.Size = { 32.f, 32.f };
    
    Enemy.FireTimer = 0.f;
    Enemy.FireDuration = FireRate; 
    Enemy.LifeDuration = LifeDuration;
    
    Enemy.FiringPatternType = FiringPatternType;
    Enemy.MoodPatternType = MoodPatternType;
    Enemy.MovementType = MovementType;
    
    // TODO: Change to appropriate enemy
    Enemy.ImageAabb = Assets->AtlasAabbs + AtlasAabb_Enemy;
    Push(&Mode->Enemies, Enemy);
}


static inline void
SpawnBullet(game_mode_main* Mode, game_assets* Assets, v2f Position, v2f Direction, f32 Speed, mood_type Mood) {
    bullet Bullet = {}; 
    Bullet.Position = Position;
	Bullet.Speed = Speed;
    Bullet.Size = { 16.f, 16.f };
    Bullet.HitCircle = {
        { 0.f, 0.f }, 
        Bullet.Size.X * 0.5f 
    };
    
    if (V2f_LengthSq(Direction) > 0.f)
	    Bullet.Direction = V2f_Normalize(Direction);
	
    Bullet.MoodType = Mood;
	Bullet.ImageAabb = Assets->AtlasAabbs + ((Bullet.MoodType == MoodType_Dot) ? AtlasAabb_BulletDot : AtlasAabb_BulletCircle);
    
    Push(&Mode->Bullets, Bullet);
}


static inline void 
InitMainMode(permanent_state* PermState,
             transient_state* TranState,
             debug_state* DebugState) 
{
    game_mode_main* Mode = PermState->MainMode;
    
    Mode->Arena = Arena_SubArena(&PermState->ModeArena, Arena_GetRemaining(PermState->ModeArena));
    Mode->Bullets = CreateList<bullet>(&Mode->Arena, 128);
    Mode->Enemies = CreateList<enemy>(&Mode->Arena, 128);
    Mode->Wave.IsDone = true;
    Mode->Rng = Seed(0); // TODO: Used system clock for seed.
    
    game_assets* Assets = TranState->Assets;
    player* Player = &Mode->Player;
    Player->Speed = 300.f;
    Player->DotImageAabb = Assets->AtlasAabbs + AtlasAabb_PlayerDot;
    Player->CircleImageAabb = Assets->AtlasAabbs + AtlasAabb_PlayerCircle;
    
    Player->Position = {};
    Player->Direction = {};
    Player->Size = v2f{ 32.f, 32.f };
	Player->HitCircle = { v2f{}, 16.f};
    
    // NOTE(Momo): We start as Dot
    Player->MoodType = MoodType_Dot;
    Player->DotImageAlpha = 1.f;
    Player->DotImageAlphaTarget = 1.f;
    
    Player->DotImageTransitionDuration = 0.05f;
    Player->DotImageTransitionTimer = Player->DotImageTransitionDuration;
    
    Mode->Wave.IsDone = true;
    
    HookU32Variable(DebugState, CreateString("Bullets"), &Mode->Bullets);
}

static inline void
UninitMainMode(debug_state* DebugState) {
    UnhookVariable(DebugState, CreateString("Bullets"));
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
    
    Player->Position += Player->Direction * Player->Speed * DeltaTime;
}

static inline void
UpdateBullet(game_mode_main* Mode,
             f32 DeltaTime) 
{
    for(usize I = 0; I < Mode->Bullets.Count;) {
        bullet* Bullet = Mode->Bullets + I;
        Bullet->Position += Bullet->Direction * Bullet->Speed * DeltaTime;
        // Out of bounds self-destruction
        if (Bullet->Position.X <= -Global_DesignSpace.W * 0.5f - Bullet->HitCircle.Radius || 
            Bullet->Position.X >= Global_DesignSpace.W * 0.5f + Bullet->HitCircle.Radius ||
            Bullet->Position.Y <= -Global_DesignSpace.H * 0.5f - Bullet->HitCircle.Radius ||
            Bullet->Position.Y >= Global_DesignSpace.H * 0.5f + Bullet->HitCircle.Radius) {
            SwapRemove(&Mode->Bullets, I);
            continue;
        }
        ++I;
    }
    
}


static inline void 
UpdateEnemies(game_mode_main* Mode,
              game_assets* Assets,
              f32 DeltaTime) 
{
    player* Player = &Mode->Player;
    for (usize I = 0; I < Mode->Enemies.Count; ++I) 
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
                Dir = Player->Position - Enemy->Position;
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
            SwapRemove(&Mode->Enemies, I);
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
    PlayerCircle.Origin += Player->Position;
    
    // Player vs every bullet
    for (usize I = 0; I < Mode->Bullets.Count;) 
    {
        bullet* Bullet = Mode->Bullets + I;
        circle2f BulletCircle = Bullet->HitCircle;
        BulletCircle.Origin += Bullet->Position;
        
        if (Circle2f_IsIntersecting(PlayerCircle, BulletCircle)) {
            if (Player->MoodType == Bullet->MoodType ) {
                SwapRemove(&Mode->Bullets, I);
                continue;
            }
        }
        ++I;
    }
}

static inline void
UpdateWaves(game_mode_main* Mode, 
            game_assets* Assets,
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
                    v2f Pos = {
                        Bilateral(&Mode->Rng) * Global_DesignSpace.W * 0.5f,
                        Bilateral(&Mode->Rng) * Global_DesignSpace.H * 0.5f
                    };
                    auto MoodType = 
                        (enemy_mood_pattern_type)Choice(&Mode->Rng, MoodType_Count);
                    
                    SpawnEnemy(Mode, 
                               Assets,
                               Pos,
                               MoodType,
                               EnemyFiringPatternType_Homing,
                               EnemyMovementType_Static,
                               0.5f, 
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
             game_assets* Assets,
             mailbox* RenderCommands) 
{
    player* Player = &Mode->Player;
    m44f S = M44f_Scale(Player->Size.X, Player->Size.Y, 1.f);
    
    v3f RenderPos = { 
        Player->Position.X, 
        Player->Position.Y, 
        ZLayPlayer 
    };
    m44f T = M44f_Translation(Player->Position.X,
                              Player->Position.Y,
                              ZLayPlayer);
    auto* Texture = GetTexture(Assets, Player->CircleImageAabb->TextureId);
    PushDrawTexturedQuad(RenderCommands, 
                         Color_White, 
                         T*S,
                         Texture->Handle,
                         GetAtlasUV(Assets, Player->CircleImageAabb));
    
    
    T = M44f_Translation(Player->Position.X,
                         Player->Position.Y,
                         ZLayPlayer + 0.01f);
    PushDrawTexturedQuad(RenderCommands, 
                         c4f{ 1.f, 1.f, 1.f, Player->DotImageAlpha}, 
                         T*S, 
                         GetTexture(Assets, Player->DotImageAabb->TextureId)->Handle,
                         GetAtlasUV(Assets, Player->DotImageAabb));
    
}

static inline void
RenderBullets(game_mode_main* Mode,
              game_assets* Assets,
              mailbox* RenderCommands) 
{
	// Bullet Rendering 
    f32 DotLayerOffset = 0.f;
    f32 CircleLayerOffset = 0.f;
    for (usize I = 0; I < Mode->Bullets.Count; ++I) 
    {
        bullet* Bullet = Mode->Bullets + I;
        
        
		m44f S = M44f_Scale(Bullet->Size.X, 
                            Bullet->Size.Y, 
                            1.f);
        
        f32 ZPos = {};
        switch(Bullet->MoodType) {
            case MoodType_Dot: {
                ZPos= ZLayDotBullet + DotLayerOffset;
                DotLayerOffset += 0.01f;
            } break;
            case MoodType_Circle: {
                ZPos = ZLayCircleBullet + CircleLayerOffset;
                CircleLayerOffset += 0.01f;
            } break;
            default: 
            Assert(false);
        }
        m44f T = M44f_Translation(Bullet->Position.X,
                                  Bullet->Position.Y,
                                  ZPos);
		PushDrawTexturedQuad(RenderCommands,
							 Color_White,
							 T*S,
							 GetTexture(Assets, Bullet->ImageAabb->TextureId)->Handle,
							 GetAtlasUV(Assets, Bullet->ImageAabb));
        
        
        
	}
}

static inline void
RenderEnemies(game_mode_main* Mode, 
              game_assets* Assets,
              mailbox* RenderCommands) 
{
    for(usize I = 0; I < Mode->Enemies.Count; ++I )
	{
        enemy* Enemy = Mode->Enemies + I;
		m44f S = M44f_Scale(Enemy->Size.X, Enemy->Size.Y, 1.f);
		v3f RenderPos = V2f_To_V3f(Enemy->Position);
		m44f T = M44f_Translation(Enemy->Position.X,
                                  Enemy->Position.Y,
                                  ZLayEnemy);
		PushDrawTexturedQuad(RenderCommands,
							 Color_White,
							 T*S,
							 GetTexture(Assets, Enemy->ImageAabb->TextureId)->Handle,
							 GetAtlasUV(Assets, Enemy->ImageAabb));
	}
}


static inline void
UpdateMainMode(permanent_state* PermState, 
               transient_state* TranState,
               mailbox* RenderCommands, 
               game_input* Input,
               f32 DeltaTime) 
{
    SwitchToGameCoords(RenderCommands);
    game_mode_main* Mode = PermState->MainMode;
    PushClearColor(RenderCommands, { 0.15f, 0.15f, 0.15f, 1.f });
    
    game_assets* Assets = TranState->Assets;
    UpdateInput(Mode, Input);
    UpdatePlayer(Mode, DeltaTime);    
    UpdateBullet(Mode, DeltaTime);
    UpdateWaves(Mode, Assets, DeltaTime);
    UpdateEnemies(Mode, Assets, DeltaTime); 
    UpdateCollision(Mode);
    
    
    RenderPlayer(Mode, Assets, RenderCommands);
    RenderBullets(Mode, Assets, RenderCommands);
    RenderEnemies(Mode, Assets, RenderCommands);
    
    
    // TODO: We would like better debug rendering system please that is more global
    // Debug Rendering 
#if REFACTOR
    {
        scratch Scratchpad(&Mode->Arena);
        string_buffer Buffer = StringBuffer(Scratchpad, 256);
        Push(&Buffer, String("Bullets: "));
        PushI32(&Buffer, (i32)Mode->Bullets.Count);
        
        // Number of dot bullets
        DrawText(RenderCommands, 
                 Assets, 
                 v3f{ -800.f + 10.f, 450.f - 32.f, 0.f }, 
                 Color_White, 
                 Font_Default, 
                 32.f, 
                 Buffer.Array);
        Clear(&Buffer);
        
        Push(&Buffer, String("Enemies: "));
        PushI32(&Buffer, (i32)Mode->Enemies.Count);
        
        DrawText(RenderCommands, 
                 Assets, 
                 v3f{ -800.f + 10.f, 450.f - 64.f, 0.f }, 
                 Color_White, 
                 Font_Default, 
                 32.f, 
                 Buffer.Array);
    }
#else 
#endif
}

#endif //GAME_MODE_H
