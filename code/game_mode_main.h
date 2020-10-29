#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "mm_easing.h"
#include "mm_maths.h"
#include "mm_swap_remove_pool.h"
#include "game.h"

enum mood_type : b32 {
    MoodType_White,
    MoodType_Black,
};

enum enemy_mood_pattern_type : u32 {
    EnemyMoodPatternType_White,
    EnemyMoodPatternType_Black,
};

enum enemy_firing_pattern_type : u32 {
    EnemyFiringPatternType_Homing,
};

enum enemy_movement_type : u32 {
    EnemyMovementType_Static,
};

struct player {
    // NOTE(Momo): Rendering
    atlas_rect* WhiteImageRect;
    atlas_rect* BlackImageRect;
    f32 WhiteImageAlpha;
    f32 WhiteImageAlphaTarget;
    f32 WhiteImageTransitionTimer;
    f32 WhiteImageTransitionDuration;
    
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

    mmsrp_pool<bullet> Bullets;
    mmsrp_pool<enemy> Enemies;

    f32 WaveTimer;
    f32 WaveDuration;

};

static inline enemy*
SpawnEnemy(game_mode_main* Mode, 
        game_assets* Assets, 
        mmm_v2f Position,
        enemy_mood_pattern_type MoodPatternType,
        enemy_firing_pattern_type FiringPatternType, 
        enemy_movement_type MovementType, 
        f32 FireRate,
        f32 LifeDuration) 
{
    enemy* Ret = mmsrp_Borrow<enemy>(&Mode->Enemies);
    
    Ret->Position = Position;
    Ret->Size = { 32.f, 32.f };

    Ret->FireTimer = 0.f;
    Ret->FireDuration = FireRate; 
    Ret->LifeDuration = LifeDuration;

    Ret->FiringPatternType = FiringPatternType;
    Ret->MoodPatternType = MoodPatternType;
    Ret->MovementType = MovementType;

    // TODO: Change to appropriate enemy
    Ret->ImageRect = Assets->AtlasRects + AtlasRect_PlayerWhite;
    
    return Ret;

}

static inline bullet*
SpawnBullet(game_mode_main* Mode, game_assets* Assets, mmm_v2f Position, mmm_v2f Direction, f32 Speed, mood_type Mood) {
    bullet* Ret = mmsrp_Borrow<bullet>(&Mode->Bullets);
    
    Ret->Position = Position;
	Ret->Speed = Speed;
    Ret->Size = { 16.f, 16.f };
    Ret->HitCircle = {
        { 0.f, 0.f }, 
        Ret->Size.X * 0.5f 
    };

    if (mmm_LengthSq(Direction) > 0.f)
	    Ret->Direction = mmm_Normalize(Direction);
	
    Ret->MoodType = Mood;
	Ret->ImageRect = Assets->AtlasRects + ((Ret->MoodType == MoodType_White) ? AtlasRect_PlayerWhite : AtlasRect_PlayerBlack);
		
	return Ret;
}
	

static inline void 
Init(game_mode_main* Mode, game_state* GameState) {
    Log("Main state initialized!");
    
    Mode->Arena = mmarn_PushArenaAll(&GameState->ModeArena);
    Mode->Bullets = mmsrp_PushPool<bullet>(&Mode->Arena, 128);
    Mode->Enemies = mmsrp_PushPool<enemy>(&Mode->Arena, 128);

    auto* Assets = GameState->Assets;
    auto* Player = &Mode->Player;
    Player->Speed = 250.f;
    Player->WhiteImageRect = Assets->AtlasRects + AtlasRect_PlayerWhite;
    Player->BlackImageRect = Assets->AtlasRects + AtlasRect_PlayerBlack;
    Player->Position = {};
    Player->Direction = {};
    Player->Size = { 64.f, 64.f };
	Player->HitCircle = {{0.f, 0.f}, 28.f};
    
    // NOTE(Momo): We start as White
    Player->MoodType = MoodType_White;
    Player->WhiteImageAlpha = 1.f;
    Player->WhiteImageAlphaTarget = 1.f;
    
    Player->WhiteImageTransitionDuration = 0.05f;
    Player->WhiteImageTransitionTimer = Player->WhiteImageTransitionDuration;
}


static inline void
Update(game_mode_main* Mode,
       game_state* GameState, 
       mmcmd_commands* RenderCommands, 
       game_input* Input,
       f32 DeltaTime) 
{
#if INTERNAL
    if (ProcessMetaInput(GameState, Input)) {
        return;
    }
#endif
    PushCommandClearColor(RenderCommands, { 0.3f, 0.3f, 0.3f, 1.f });
    PushCommandSetOrthoBasis(RenderCommands, {}, { 1600.f, 900.f, 1000.f });
    
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
            Player->MoodType = (Player->MoodType == MoodType_White) ? MoodType_Black : MoodType_White;
            
            switch(Player->MoodType) {
                case MoodType_White: {
                    Player->WhiteImageAlphaTarget = 1.f;
                } break;
                case MoodType_Black: {
                    Player->WhiteImageAlphaTarget = 0.f;
                }break;
                
            }
            Player->WhiteImageTransitionTimer = 0.f;
        }
    }
    
    // NOTE(Momo): Player Update
    {
        
        Player->WhiteImageAlpha = Lerp(1.f - Player->WhiteImageAlphaTarget, 
                                       Player->WhiteImageAlphaTarget, Player->WhiteImageTransitionTimer / Player->WhiteImageTransitionDuration);
        
        Player->WhiteImageTransitionTimer += DeltaTime;
        Player->WhiteImageTransitionTimer = Clamp(Player->WhiteImageTransitionTimer, 0.f, Player->WhiteImageTransitionDuration);
        
        Player->Position += Player->Direction * Player->Speed * DeltaTime;
    }


	// Bullet Update
	for( u32 i = 0; i < Mode->Bullets.Used; ++i) {
		bullet* Bullet = &Mode->Bullets[i];
		Bullet->Position += Bullet->Direction * Bullet->Speed * DeltaTime;
	}


    // Spawning logic Update
    {
        // TODO: Wave logic
        Mode->WaveTimer += DeltaTime;
        if (Mode->WaveTimer >= Mode->WaveDuration) {
            // randomize a wave logic
            // For now just create an enemy at a location
            SpawnEnemy(Mode, 
                    Assets, 
                    {}, 
                    EnemyMoodPatternType_White,
                    EnemyFiringPatternType_Homing,
                    EnemyMovementType_Static,
                    3.0f,
                    10.f);
            
            Mode->WaveTimer = 0.f;
        }


    }

	// Enemy logic update
    for ( u32 i = 0; i < Mode->Enemies.Used; ++i )
	{
        enemy* Enemy = &Mode->Enemies[i];

        // Tick all timers

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
            mmm_v2f Dir = Player->Position - Enemy->Position;
            switch (Enemy->FiringPatternType) {
		    	case EnemyFiringPatternType_Homing: 
					SpawnBullet(Mode, Assets, Enemy->Position, Dir, 100.f, MoodType_White);
			        break;
                default:
                    Assert(false);
		    }

            Enemy->FireTimer = 0.f;
		}

        // Life time
        Enemy->LifeTimer += DeltaTime;
        if (Enemy->LifeTimer > Enemy->LifeDuration) {
            mmsrp_Return(&Mode->Enemies, Enemy);
            --i;
        }

	}
   
	// Collision 
	{
		// Player vs every bullet
		for( u32 i = 0; i < Mode->Bullets.Used; ++i) {
            bullet* Bullet = &Mode->Bullets[i];
            mmm_circle2f PlayerCircle = Player->HitCircle;
            PlayerCircle.Origin += Player->Position;

            mmm_circle2f BulletCircle = Bullet->HitCircle;
            BulletCircle.Origin += Bullet->Position;

            if (mmm_IsIntersecting(PlayerCircle, BulletCircle)) {
                 if (Player->MoodType == Bullet->MoodType ) {
                    mmsrp_Return(&Mode->Bullets, Bullet);
                    --i;
                 }
            }
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
                                    Player->BlackImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->BlackImageRect));
        
		RenderPos.Z += 0.1f;
        T = mmm_Translation(RenderPos);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, Player->WhiteImageAlpha}, 
                                    T*S, 
                                    Player->WhiteImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->WhiteImageRect));
    }

	// Bullet Rendering 
	for( u32 i = 0; i < Mode->Bullets.Used; ++i) 
	{
		bullet* Bullet = &Mode->Bullets[i];
    
		mmm_m44f S = mmm_Scale(mmm_V3F(Bullet->Size));
		mmm_v3f RenderPos = mmm_V3F(Bullet->Position);
        switch(Bullet->MoodType) {
            case MoodType_White:
                RenderPos.Z = ZLayDotBullet + (f32)i * 0.01f;
                break;
            case MoodType_Black:
                RenderPos.Z = ZLayCircleBullet + (f32)i * 0.01f; 
                break;
        }
        mmm_m44f T = mmm_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Bullet->ImageRect->BitmapId,
									GetAtlasUV(Assets, Bullet->ImageRect));
	}


	// Enemy Rendering
    for( u32 i = 0; i < Mode->Enemies.Used; ++i )
	{
	    enemy* Enemy = &Mode->Enemies[i];
		mmm_m44f S = mmm_Scale(mmm_V3F(Enemy->Size));
		mmm_v3f RenderPos = mmm_V3F(Enemy->Position);
		RenderPos.Z = ZLayEnemy; 
		mmm_m44f T = mmm_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Enemy->ImageRect->BitmapId,
									GetAtlasUV(Assets, Enemy->ImageRect));
	}


}

#endif //GAME_MODE_H
