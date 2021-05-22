/* date = May 14th 2021 10:44 am */

#ifndef GAME_MODE_MAIN_SPAWN_H
#define GAME_MODE_MAIN_SPAWN_H


static inline void
SpawnEnemy(game_mode_main* Mode, 
           assets* Assets, 
           v2f Position,
           enemy_mood_pattern_type MoodPatternType,
           enemy_firing_pattern_type FiringPatternType, 
           enemy_movement_type MovementType) 
{
    enemy* Enemy = List_Push(&Mode->Enemies);
    Enemy->Position = Position;
    Enemy->Size = V2f_Create(32.f, 32.f);
    
    Enemy->FireTimer = 0.f;
    Enemy->LifeTimer = 0.f;
    
    Enemy->FiringPatternType = FiringPatternType;
    Enemy->MoodPatternType = MoodPatternType;
    Enemy->MovementType = MovementType;
    Enemy->State = EnemyState_Spawning;
    
}

static inline void
SpawnParticleRandomDirectionAndSpeed(game_mode_main* Mode, 
                                     assets* Assets, 
                                     v2f Position,
                                     u32 Amount) 
{
    for (u32 I = 0; I < Amount; ++I) {
        particle* P = Queue_Push(&Mode->Particles);
        if (!P) {
            return;
        }
        P->Position = Position;
        P->Direction.X = Rng_Bilateral(&Mode->Rng);
        P->Direction.Y = Rng_Bilateral(&Mode->Rng);
        P->Direction = V2f_Normalize(P->Direction);
        P->Timer = 0.f;
        P->Speed = Rng_Between(&Mode->Rng, P->SpeedMin, P->SpeedMax);
    }
    
    
}

static inline void
SpawnBullet(game_mode_main* Mode, 
            assets* Assets, 
            v2f Position, 
            v2f Direction, 
            f32 Speed, 
            mood_type Mood) 
{
    bullet* B = nullptr;
    switch (Mood) {
        case MoodType_Dot: {
            B = List_Push(&Mode->DotBullets);
        } break;
        case MoodType_Circle: {
            B = List_Push(&Mode->CircleBullets);
        } break;
        default: {
            Assert(false);
        }
    }
    B->Position = Position;
	B->Speed = Speed;
    B->Size = V2f_Create(16.f, 16.f);
    
    B->HitCircle = Circle2f_Create(V2f_Create(0.f, 0.f), 4.f);
    
    if (V2f_LengthSq(Direction) > 0.f) {
	    B->Direction = V2f_Normalize(Direction);
    }
    B->MoodType = Mood;
    
}

#endif //GAME_MODE_MAIN_SPAWN_H
