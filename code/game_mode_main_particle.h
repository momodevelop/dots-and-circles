/* date = May 22nd 2021 7:10 pm */

#ifndef GAME_MODE_MAIN_PARTICLE_H
#define GAME_MODE_MAIN_PARTICLE_H

struct particle {
    constexpr static f32 Duration = 0.25f;
    constexpr static f32 Alpha = 0.8f;
    constexpr static f32 Size = 10.f;
    constexpr static f32 SpeedMin = 10.f;
    constexpr static f32 SpeedMax = 20.f;
    
    f32 Timer;
    v2f Position;
    v2f Direction;
    f32 Speed;
};


#endif //GAME_MODE_MAIN_PARTICLE_H
