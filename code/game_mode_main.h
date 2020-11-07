#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "mm_easing.h"
#include "mm_maths.h"
#include "mm_random.h"
#include "mm_unordered_list.h"
#include "game.h"

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
    atlas_rect* DotImageRect;
    atlas_rect* CircleImageRect;
    f32 DotImageAlpha;
    f32 DotImageAlphaTarget;
    f32 DotImageTransitionTimer;
    f32 DotImageTransitionDuration;
    
    mmm_v2f Size;
   
	// Collision
    mmm_circle2f HitCircle;


    // Physics
    mmm_v2f Position;
    mmm_v2f Direction; 
    
    // Gameplay
    mood_type MoodType;
    f32 Speed;
};


struct bullet {
	atlas_rect* ImageRect;
	mmm_v2f Size;
    mood_type MoodType;
    mmm_v2f Direction;
	mmm_v2f Position;
	f32 Speed;
	mmm_circle2f HitCircle; 
};

struct enemy {
    atlas_rect* ImageRect;
	mmm_v2f Size; 
	mmm_v2f Position;
    enemy_firing_pattern_type FiringPatternType;
    enemy_mood_pattern_type MoodPatternType;
    enemy_movement_type MovementType;

    f32 FireTimer;
	f32 FireDuration;

    f32 LifeTimer;
    f32 LifeDuration;
};

struct game_mode_main {
    mmarn_arena Arena;

    player Player;

    mmul_list<bullet> Bullets;
    mmul_list<enemy> Enemies;

    wave Wave;
    mmrng_series Rng;
};

static inline void
SpawnEnemy(game_mode_main* Mode, 
        game_assets* Assets, 
        mmm_v2f Position,
        enemy_mood_pattern_type MoodPatternType,
        enemy_firing_pattern_type FiringPatternType, 
        enemy_movement_type MovementType, 
        f32 FireRate,
        f32 LifeDuration) 
{
    Log("Enemy spawned at: %f, %f!", Position.X, Position.Y);
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
    Enemy.ImageRect = Assets->AtlasRects + AtlasRect_PlayerDot;
    mmul_Add(&Mode->Enemies, Enemy);
}


static inline void
SpawnBullet(game_mode_main* Mode, game_assets* Assets, mmm_v2f Position, mmm_v2f Direction, f32 Speed, mood_type Mood) {
    bullet Bullet = {}; 
    Bullet.Position = Position;
	Bullet.Speed = Speed;
    Bullet.Size = { 16.f, 16.f };
    Bullet.HitCircle = {
        { 0.f, 0.f }, 
        Bullet.Size.X * 0.5f 
    };

    if (mmm_LengthSq(Direction) > 0.f)
	    Bullet.Direction = mmm_Normalize(Direction);
	
    Bullet.MoodType = Mood;
	Bullet.ImageRect = Assets->AtlasRects + ((Bullet.MoodType == MoodType_Dot) ? AtlasRect_PlayerDot : AtlasRect_PlayerCircle);
		
    mmul_Add(&Mode->Bullets, Bullet);
}
	

static inline void 
Init(game_mode_main* Mode, game_state* GameState) {
    Log("Main state initialized!");
    
    Mode->Arena = mmarn_PushArenaAll(&GameState->ModeArena);
    Mode->Bullets = mmul_PushList<bullet>(&Mode->Arena, 128);
    Mode->Enemies = mmul_PushList<enemy>(&Mode->Arena, 128);
    Mode->Wave.IsDone = true;
    Mode->Rng = mmrng_Seed(0); // TODO: Used system clock for seed.

    auto* Assets = GameState->Assets;
    auto* Player = &Mode->Player;
    Player->Speed = 250.f;
    Player->DotImageRect = Assets->AtlasRects + AtlasRect_PlayerDot;
    Player->CircleImageRect = Assets->AtlasRects + AtlasRect_PlayerCircle;
    Player->Position = {};
    Player->Direction = {};
    Player->Size = { 64.f, 64.f };
	Player->HitCircle = {{0.f, 0.f}, 28.f};
    
    // NOTE(Momo): We start as Dot
    Player->MoodType = MoodType_Dot;
    Player->DotImageAlpha = 1.f;
    Player->DotImageAlphaTarget = 1.f;
    
    Player->DotImageTransitionDuration = 0.05f;
    Player->DotImageTransitionTimer = Player->DotImageTransitionDuration;
    
    Mode->Wave.IsDone = true;

}


static inline void
Update(game_mode_main* Mode,
       game_state* GameState, 
       mmcmd_commands* RenderCommands, 
       game_input* Input,
       f32 DeltaTime) 
{
    constexpr static f32 DesignWidth = 1600.f;
    constexpr static f32 DesignHeight = 900.f;
    constexpr static f32 DesignDepth = 1000.f;

    PushCommandClearColor(RenderCommands, { 0.15f, 0.15f, 0.15f, 1.f });
    PushCommandSetOrthoBasis(RenderCommands, {}, { DesignWidth, DesignHeight, DesignDepth });
    
    auto* Assets = GameState->Assets;
    auto* Player = &Mode->Player;
    
    // NOTE(Momo): Input
    {
        mmm_v2f Direction = {};
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
            Player->Direction = mmm_Normalize(Direction);
        else {
            Player->Direction = {};
        }
        
        
        // NOTE(Momo): Absorb Mode Switch
        if(IsPoked(Input->ButtonSwitch)) {
            Player->MoodType = (Player->MoodType == MoodType_Dot) ? MoodType_Circle : MoodType_Dot;
            
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
    
    // NOTE(Momo): Player Update
    {
        
        Player->DotImageAlpha = Lerp(1.f - Player->DotImageAlphaTarget, 
                Player->DotImageAlphaTarget, 
                Player->DotImageTransitionTimer / Player->DotImageTransitionDuration);
        
        Player->DotImageTransitionTimer += DeltaTime;
        Player->DotImageTransitionTimer = Clamp(Player->DotImageTransitionTimer, 0.f, Player->DotImageTransitionDuration);
        
        Player->Position += Player->Direction * Player->Speed * DeltaTime;
    }


	// Bullet Update
	for( auto It = mmul_Begin(&Mode->Bullets); It != mmul_End(&Mode->Bullets); ++It) {
		It->Position += It->Direction * It->Speed * DeltaTime;
	}


    // Wave logic Update
    {
        if (Mode->Wave.IsDone) {
            // TODO: Random wave type
            Mode->Wave.Type = WavePatternType_SpawnNForDuration;
            Log("Wave started!"); 
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
                        mmm_v2f Pos = {
                            mmrng_Bilateral(&Mode->Rng) * DesignWidth * 0.5f,
                            mmrng_Bilateral(&Mode->Rng) * DesignHeight * 0.5f
                        };
                        auto MoodType = (enemy_mood_pattern_type)mmrng_Choice(&Mode->Rng, MoodType_Count);
                        Log("%d\n", MoodType);
                        SpawnEnemy(Mode, Assets,
                                Pos,
                                MoodType,
                                EnemyFiringPatternType_Homing,
                                EnemyMovementType_Static,
                                1.5f, 10.f);

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

	// Enemy logic update
    for ( auto It = mmul_Begin(&Mode->Enemies); It != mmul_End(&Mode->Enemies);) 
    {
        // Movement
        switch( It->MovementType ) {
            case EnemyMovementType_Static:
                // Do nothing
                break;
            default: 
                Assert(false);
        }

        // Fire
        It->FireTimer += DeltaTime;
		if (It->FireTimer > It->FireDuration) {       
            mood_type MoodType = {};
            switch (It->MoodPatternType) {
                case EnemyMoodPatternType_Dot: 
                    MoodType = MoodType_Dot;
                    break;
                case EnemyMoodPatternType_Circle:
                    MoodType = MoodType_Circle;
                    break;
                default:
                    Assert(false);
            }

            mmm_v2f Dir = {};
            switch (It->FiringPatternType) {
		    	case EnemyFiringPatternType_Homing: 
                    Dir = Player->Position - It->Position;
			        break;
                default:
                    Assert(false);
		    }
            SpawnBullet(Mode, Assets, It->Position, Dir, 100.f, MoodType);
            It->FireTimer = 0.f;
		}

        // Life time
        It->LifeTimer += DeltaTime;
        if (It->LifeTimer > It->LifeDuration) {
            It = mmul_Remove(&Mode->Enemies, It);
            continue;
        }
        ++It;
	}
   
	// Collision 
	{
        mmm_circle2f PlayerCircle = Player->HitCircle;
        PlayerCircle.Origin += Player->Position;

		// Player vs every bullet
		//for( u32 i = 0; i < Mode->Bullets.Used; ++i) {
        mmul_list<bullet>* Bullets = &Mode->Bullets;
        for ( auto It = mmul_Begin(Bullets); It != mmul_End(Bullets);) 
        {
            mmm_circle2f BulletCircle = It->HitCircle;
            BulletCircle.Origin += It->Position;

            if (mmm_IsIntersecting(PlayerCircle, BulletCircle)) {
                 if (Player->MoodType == It->MoodType ) {
                    It = mmul_Remove(&Mode->Bullets, It);
                    continue;
                 }
            }

            ++It;
		}
	}


    constexpr static f32 ZLayPlayer =  0.f;
    constexpr static f32 ZLayDotBullet = 100.f;
    constexpr static f32 ZLayCircleBullet = 200.f;
    constexpr static f32 ZLayEnemy = 300.f;
    // NOTE(Momo): Player Rendering
    {
        mmm_m44f S = mmm_Scale(mmm_V3F(Player->Size));
        
		mmm_v3f RenderPos = mmm_V3F(Player->Position);
        RenderPos.Z = ZLayPlayer;

        mmm_m44f T = mmm_Translation(RenderPos);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, 1.f }, 
                                    T*S, 
                                    Player->CircleImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->CircleImageRect));
        
		RenderPos.Z += 0.1f;
        T = mmm_Translation(RenderPos);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, Player->DotImageAlpha}, 
                                    T*S, 
                                    Player->DotImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->DotImageRect));
    }

	// Bullet Rendering 
    f32 DotLayerOffset = 0.f;
    f32 CircleLayerOffset = 0.f;
	for( auto&& Bullet : Mode->Bullets) 
    {
		mmm_m44f S = mmm_Scale(mmm_V3F(Bullet.Size));
		mmm_v3f RenderPos = mmm_V3F(Bullet.Position);
        switch(Bullet.MoodType) {
            case MoodType_Dot:
                RenderPos.Z = ZLayDotBullet + DotLayerOffset;
                DotLayerOffset += 0.01f;
                break;
            case MoodType_Circle:
                RenderPos.Z = ZLayCircleBullet + CircleLayerOffset;
                CircleLayerOffset += 0.01f;
                break;
            default: 
                Assert(false);
        }
        mmm_m44f T = mmm_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Bullet.ImageRect->BitmapId,
									GetAtlasUV(Assets, Bullet.ImageRect));

    
	}


	// Enemy Rendering
    for(auto&& Enemy : Mode->Enemies )
	{
		mmm_m44f S = mmm_Scale(mmm_V3F(Enemy.Size));
		mmm_v3f RenderPos = mmm_V3F(Enemy.Position);
		RenderPos.Z = ZLayEnemy; 
		mmm_m44f T = mmm_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Enemy.ImageRect->BitmapId,
									GetAtlasUV(Assets, Enemy.ImageRect));
	}


}

#endif //GAME_MODE_H
