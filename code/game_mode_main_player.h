/* date = May 22nd 2021 7:12 pm */

#ifndef GAME_MODE_MAIN_PLAYER_H
#define GAME_MODE_MAIN_PLAYER_H

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
    v2f PrevPosition;
    
    // Gameplay
    mood_type MoodType;
};

#endif //GAME_MODE_MAIN_PLAYER_H
