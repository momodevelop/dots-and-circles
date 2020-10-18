#ifndef GAME_MODE_MAIN_H
#define GAME_MODE_MAIN_H

#include "game.h"
#include "ryoji_easing.h"
#include "ryoji_maths.h"

enum mood_type : b32 {
    MoodType_White,
    MoodType_Black,
};

enum bullet_pattern_type : u32 {
    BulletPatternType_Homing,
};

struct player {
    // NOTE(Momo): Rendering
    atlas_rect* WhiteImageRect;
    atlas_rect* BlackImageRect;
    f32 WhiteImageAlpha;
    f32 WhiteImageAlphaTarget;
    f32 WhiteImageTransitionTimer;
    f32 WhiteImageTransitionDuration;
    
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
    bullet_pattern_type BulletPatternType;

    f32 FireTimer;
	f32 FireDuration;

};

struct game_mode_main {
    player Player;
	
	bullet Bullets[10];
	u32 BulletCount;
    
	enemy Enemy;

};

static inline bullet*
SpawnBullet(game_mode_main* Mode, game_assets* Assets, v2f Position, v2f Direction, f32 Speed, mood_type Mood) {
	Assert(Mode->BulletCount < ArrayCount(Mode->Bullets));
    bullet* Ret = Mode->Bullets + Mode->BulletCount;
    Ret->Position = Position;
	Ret->Speed = Speed;
	if (LengthSq(Direction) > 0.f)
	    Ret->Direction = Normalize(Direction);
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


	for (u32 i = 0; i < ArrayCount(Mode->Bullets); ++i) {
	    bullet* Bullet = Mode->Bullets + i;
		Bullet->Direction = {};
		Bullet->Position = {};
		Bullet->Speed = 0.f;
		Bullet->Size = { 32.f, 32.f };
		Bullet->HitCircle = {{ 0.f, 0.f }, 12.f};
	}
    
    // Test enemy
	enemy* Enemy = &Mode->Enemy;
    Enemy->Position = { 100, -100 };
    Enemy->ImageRect = Assets->AtlasRects + AtlasRect_PlayerWhite;
	Enemy->Size = { 48.f, 48.f };
	Enemy->FireTimer = 0.f;
	Enemy->FireDuration = 1.f;

}

static inline void
Collide(game_mode_main* Mode, player* Player, bullet* Bullet) {
    circle2f PlayerCircle = Player->HitCircle;
	PlayerCircle.Origin += Player->Position;

	circle2f BulletCircle = Bullet->HitCircle;
	BulletCircle.Origin += Bullet->Position;

	if (IsIntersecting(PlayerCircle, BulletCircle)) {
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
       commands* RenderCommands, 
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
	{
        enemy* Enemy = &Mode->Enemy;
        Enemy->FireTimer += DeltaTime;

		if (Enemy->FireTimer > Enemy->FireDuration) {
	        switch (Enemy->BulletPatternType) {
		    	case BulletPatternType_Homing: {
		            v2f Dir = Player->Position - Enemy->Position;
					SpawnBullet(Mode, Assets, Enemy->Position, Dir, 100.f, MoodType_White);
			    } break;
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
        m44f S = ScaleMatrix(V3F(Player->Size));
        
		v3f RenderPos = V3F(Player->Position);
        RenderPos.Z = ZLay_Player;

        m44f T = TranslationMatrix(RenderPos);
        PushCommandDrawTexturedQuad(RenderCommands, 
                                    {1.f, 1.f, 1.f, 1.f }, 
                                    T*S, 
                                    Player->BlackImageRect->BitmapId,
                                    GetAtlasUV(Assets, Player->BlackImageRect));
        
		RenderPos.Z += 0.1f;
        T = TranslationMatrix(RenderPos);
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

		m44f S = ScaleMatrix(V3F(Bullet->Size));
		v3f RenderPos = V3F(Bullet->Position);
		RenderPos.Z = ZLay_Bullet + (f32)i * 0.01f;
		m44f T = TranslationMatrix(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Bullet->ImageRect->BitmapId,
									GetAtlasUV(Assets, Bullet->ImageRect));
	}


	// Enemy Rendering
	{
	    enemy* Enemy = &Mode->Enemy;
		m44f S = ScaleMatrix(V3F(Enemy->Size));
		v3f RenderPos = V3F(Enemy->Position);
		RenderPos.Z = ZLay_Enemy; 
		m44f T = TranslationMatrix(RenderPos);
		PushCommandDrawTexturedQuad(RenderCommands,
									{ 1.f, 1.f, 1.f, 1.f },
									T*S,
									Enemy->ImageRect->BitmapId,
									GetAtlasUV(Assets, Enemy->ImageRect));
	}


}

#endif //GAME_MODE_H
