/* date = May 22nd 2021 7:11 pm */

#ifndef GAME_MODE_MAIN_BULLET_H
#define GAME_MODE_MAIN_BULLET_H


struct bullet {
    v2f Size;
    mood_type MoodType;
    v2f Direction;
	v2f Position;
	f32 Speed;
	circle2f HitCircle; 
};


#endif //GAME_MODE_MAIN_BULLET_H
