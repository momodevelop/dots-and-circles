#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "game.h"
#include "mm_easing.h"

#include "mm_maths.h"

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
    player Player;
	
	bullet Bullets[128];
	u32 BulletCount;
    
	enemy Enemies[128];
    u32 EnemyCount;

    // Manages spawning of enemies



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
    Assert(Mode->EnemyCount < ArrayCount(Mode->Enemies));
    enemy* Ret = Mode->Enemies + Mode->EnemyCount;
    
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
    
    ++Mode->EnemyCount;
    return Ret;

}

static inline void 
DestroyEnemy(game_mode_main* Mode, enemy* Enemy) {
    // swap with active enemy at the end
    *Enemy = Mode->Enemies[Mode->EnemyCount];
    --Mode->EnemyCount;
}

static inline bullet*
SpawnBullet(game_mode_main* Mode, game_assets* Assets, mmm_v2f Position, mmm_v2f Direction, f32 Speed, mood_type Mood) {
	Assert(Mode->BulletCount < ArrayCount(Mode->Bullets));
    bullet* Ret = Mode->Bullets + Mode->BulletCount;
    
    Ret->Position = Position;
	Ret->Speed = Speed;
    Ret->Size = { 32.f, 32.f };
    Ret->HitCircle = {{ 0.f, 0.f }, 12.f };

    if (mmm_LengthSq(Direction) > 0.f)
	    Ret->Direction = mmm_Normalize(Direction);
	Ret->MoodType = Mood;
	Ret->ImageRect = Assets->AtlasRects + ((Ret->MoodType == MoodType_White) ? AtlasRect_PlayerWhite : AtlasRect_PlayerBlack);
	++Mode->BulletCount;
		
	return Ret;
}
	
static inline void
DestroyBullet(game_mode_main* Mode, bullet* Bullet) {
    // Swap with active bullet at the end
    *Bullet = Mode->Bullets[Mode->BulletCount-1];	
	--Mode->BulletCount;
}

static inline void 
Init(game_mode_main* Mode, game_state* GameState) {
    Log("Main state initialized!");
	Mode->BulletCount = 0;
			
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

	for (u32 i = 0; i < ArrayCount(Mode->Bullets); ++i) 
        Mode->Bullets[i] = {};
    
    for (u32 i = 0; i < ArrayCount(Mode->Enemies); ++i) 
        Mode->Enemies[i] = {};
}

static inline void
Collide(game_mode_main* Mode, player* Player, bullet* Bullet) {
    mmm_circle2f PlayerCircle = Player->HitCircle;
	PlayerCircle.Origin += Player->Position;

	mmm_circle2f BulletCircle = Bullet->HitCircle;
	BulletCircle.Origin += Bullet->Position;

	if (mmm_IsIntersecting(PlayerCircle, BulletCircle)) {
	     if (Player->MoodType == Bullet->MoodType ) {
		     Log("We Vibing!");
		     DestroyBullet(Mode, Bullet);
		 }
		 else {
		     Log("Emo :(");
		 }
	}
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
    PushCommandSetOrthoBasis(RenderCommands, {}, { 1600.f, 900.f, 200.f });
    
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
	for( u32 i = 0; i < Mode->BulletCount; ++i) {
		bullet* Bullet = Mode->Bullets + i;
		Bullet->Position += Bullet->Direction * Bullet->Speed * DeltaTime;
	}


	// Enemy update
    for ( u32 i = 0; i < Mode->EnemyCount; ++i )
	{
        enemy* Enemy = Mode->Enemies + i;
        Enemy->FireTimer += DeltaTime;

        // Movement
        switch( Enemy->MovementType ) {
            case EnemyMovementType_Static:
                // Do nothing
                break;
            default: 
                Assert(false);
        }
        // Fire
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

	}
   
	// Collision 
	{
		// Player vs every bullet
		for( u32 i = 0; i < Mode->BulletCount; ++i) {
            bullet* Bullet = Mode->Bullets + i;
			Collide(Mode, Player, Bullet);
		}
	}


#define ZLay_Player 0.f
#define ZLay_Bullet 50.f
#define ZLay_Enemy  100.f
    // NOTE(Momo): Player Rendering
    {
        mmm_m44f S = mmm_Scale(mmm_V3F(Player->Size));
        
		mmm_v3f RenderPos = mmm_V3F(Player->Position);
        RenderPos.Z = ZLay_Player;

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
	for( u32 i = 0; i < Mode->BulletCount; ++i) 
	{
		bullet* Bullet = Mode->Bullets + i;

		mmm_m44f S = mmm_Scale(mmm_V3F(Bullet->Size));
		mmm_v3f RenderPos = mmm_V3F(Bullet->Position);
		RenderPos.Z = ZLay_Bullet + (f32)i * 0.01f;
		mmm_m44f T = mmm_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Bullet->ImageRect->BitmapId,
									GetAtlasUV(Assets, Bullet->ImageRect));
	}


	// Enemy Rendering
    for( u32 i = 0; i < Mode->EnemyCount; ++i )
	{
	    enemy* Enemy = Mode->Enemies + i;
		mmm_m44f S = mmm_Scale(mmm_V3F(Enemy->Size));
		mmm_v3f RenderPos = mmm_V3F(Enemy->Position);
		RenderPos.Z = ZLay_Enemy; 
		mmm_m44f T = mmm_Translation(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Enemy->ImageRect->BitmapId,
									GetAtlasUV(Assets, Enemy->ImageRect));
	}


}

#endif //GAME_MODE_H
