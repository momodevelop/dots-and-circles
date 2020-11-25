#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "mm_easing.h"
#include "mm_maths.h"
#include "mm_random.h"
#include "mm_list.h"
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
	atlas_rect* ImageRect;
	v2f Size;
    mood_type MoodType;
    v2f Direction;
	v2f Position;
	f32 Speed;
	circle2f HitCircle; 
};

struct enemy {
    atlas_rect* ImageRect;
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
    Enemy.ImageRect = Assets->AtlasRects + AtlasRect_PlayerDot;
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

    if (LengthSq(Direction) > 0.f)
	    Bullet.Direction = Normalize(Direction);
	
    Bullet.MoodType = Mood;
	Bullet.ImageRect = Assets->AtlasRects + ((Bullet.MoodType == MoodType_Dot) ? AtlasRect_PlayerDot : AtlasRect_PlayerCircle);
		
    Push(&Mode->Bullets, Bullet);
}
	

static inline void 
InitMainMode(game_state* GameState) {
    game_mode_main* Mode = GameState->MainMode;

    Mode->Arena = SubArena(&GameState->ModeArena, Remaining(GameState->ModeArena));
    Mode->Bullets = List<bullet>(&Mode->Arena, 128);
    Mode->Enemies = List<enemy>(&Mode->Arena, 128);
    Mode->Wave.IsDone = true;
    Mode->Rng = Seed(0); // TODO: Used system clock for seed.

    auto* Assets = &GameState->Assets;
    auto* Player = &Mode->Player;
    Player->Speed = 300.f;
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
UpdateMainMode(game_state* GameState, 
       mmcmd_commands* RenderCommands, 
       game_input* Input,
       f32 DeltaTime) 
{
    game_mode_main* Mode = GameState->MainMode;
    PushCommandClearColor(RenderCommands, { 0.15f, 0.15f, 0.15f, 1.f });
    PushCommandOrthoCamera(RenderCommands, 
            V3F(), 
            Rect3F( V3F(DesignWidth, DesignHeight, DesignDepth), V3F(0.5f, 0.5f, 0.5f))
    );
    
    auto* Assets = &GameState->Assets;
    auto* Player = &Mode->Player;
    
    // NOTE(Momo): Input
    {
        v2f Direction = {};
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
            Player->Direction = Normalize(Direction);
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
	for(usize i = 0; i < Mode->Bullets.Length; ++i) {
        bullet* Bullet = Mode->Bullets + i;
		Bullet->Position += Bullet->Direction * Bullet->Speed * DeltaTime;
	}


    // Wave logic Update
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
                            Bilateral(&Mode->Rng) * DesignWidth * 0.5f,
                            Bilateral(&Mode->Rng) * DesignHeight * 0.5f
                        };
                        auto MoodType = (enemy_mood_pattern_type)Choice(&Mode->Rng, MoodType_Count);
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
    for (usize I = 0; I < Mode->Enemies.Length; ++I) 
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
   
	// Collision 
	{
        circle2f PlayerCircle = Player->HitCircle;
        PlayerCircle.Origin += Player->Position;

		// Player vs every bullet
        for (usize I = 0; I < Mode->Bullets.Length; ++I) 
        {
            bullet* Bullet = Mode->Bullets + I;
            circle2f BulletCircle = Bullet->HitCircle;
            BulletCircle.Origin += Bullet->Position;

            if (IsIntersecting(PlayerCircle, BulletCircle)) {
                 if (Player->MoodType == Bullet->MoodType ) {
                    SwapRemove(&Mode->Bullets, I);
                    continue;
                 }
            }

            ++Bullet;
		}
	}

    // Rendering Logic
    constexpr static f32 ZLayPlayer =  0.f;
    constexpr static f32 ZLayDotBullet = 10.f;
    constexpr static f32 ZLayCircleBullet = 20.f;
    constexpr static f32 ZLayEnemy = 30.f;
    
    // Player Rendering
    {
        m44f S = M44F_Scale(V3F(Player->Size));
        
		v3f RenderPos = V3F(Player->Position);
        RenderPos.Z = ZLayPlayer;

        m44f T = M44F_Translation(RenderPos);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, 1.f }, 
                                    T*S, 
                                    Player->CircleImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->CircleImageRect));
        
		RenderPos.Z += 0.1f;
        T = M44F_Translation(RenderPos);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, Player->DotImageAlpha}, 
                                    T*S, 
                                    Player->DotImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->DotImageRect));
    }

	// Bullet Rendering 
    f32 DotLayerOffset = 0.f;
    f32 CircleLayerOffset = 0.f;
    for (usize I = 0; I < Mode->Bullets.Length; ++I) 
    {
        bullet* Bullet = Mode->Bullets + I;

		m44f S = M44F_Scale(V3F(Bullet->Size));
		v3f RenderPos = V3F(Bullet->Position);
        switch(Bullet->MoodType) {
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
        m44f T = M44F_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Bullet->ImageRect->BitmapId,
									GetAtlasUV(Assets, Bullet->ImageRect));

    
	}


	// Enemy Rendering
    for(usize I = 0; I < Mode->Enemies.Length; ++I )
	{
        enemy* Enemy = Mode->Enemies + I;
		m44f S = M44F_Scale(V3F(Enemy->Size));
		v3f RenderPos = V3F(Enemy->Position);
		RenderPos.Z = ZLayEnemy; 
		m44f T = M44F_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Enemy->ImageRect->BitmapId,
									GetAtlasUV(Assets, Enemy->ImageRect));
	}


}

#endif //GAME_MODE_H
