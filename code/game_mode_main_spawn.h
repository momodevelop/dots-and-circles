/* date = May 14th 2021 10:44 am */

#ifndef GAME_MODE_MAIN_SPAWN_H
#define GAME_MODE_MAIN_SPAWN_H


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
    enemy* Enemy = List_Push(&Mode->Enemies);
    Enemy->Position = Position;
    Enemy->Size = V2f_Create(32.f, 32.f);
    
    Enemy->FireTimer = 0.f;
    Enemy->FireDuration = FireRate; 
    Enemy->LifeTimer = 0.f;
    Enemy->LifeDuration = LifeDuration;
    
    Enemy->FiringPatternType = FiringPatternType;
    Enemy->MoodPatternType = MoodPatternType;
    Enemy->MovementType = MovementType;
    
    
}

static inline void
SpawnParticleRandomDirectionAndSpeed(game_mode_main* Mode, 
                                     assets* Assets, 
                                     v2f Position) 
{
    particle* P = Queue_Push(&Mode->Particles);
    P->Position = Position;
    P->Direction.X = Rng_Bilateral(&Mode->Rng);
    P->Direction.Y = Rng_Bilateral(&Mode->Rng);
    P->Direction = V2f_Normalize(P->Direction);
    
    P->Speed = Rng_Between(&Mode->Rng, 1.f, 3.f);
    
    
}

static inline void
SpawnBullet(game_mode_main* Mode, 
            assets* Assets, 
            v2f Position, 
            v2f Direction, 
            f32 Speed, 
            mood_type Mood) 
{
    bullet* B = Null;
    switch (Mood) {
        case MoodType_Dot: {
            B = List_Push(&Mode->DotBullets);
        } break;
        case MoodType_Circle: {
            B = List_Push(&Mode->CircleBullets);
        } break;
        default: {
            Assert(False);
        }
    }
    B->Position = Position;
	B->Speed = Speed;
    B->Size = V2f_Create(16.f, 16.f);
    
    B->HitCircle = Circle2f_Create(V2f_Create(0.f, 0.f), 
                                   B->Size.X * 0.5f);
    
    if (V2f_LengthSq(Direction) > 0.f) {
	    B->Direction = V2f_Normalize(Direction);
    }
    B->MoodType = Mood;
    
}

#endif //GAME_MODE_MAIN_SPAWN_H
