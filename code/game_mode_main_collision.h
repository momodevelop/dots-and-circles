/* date = May 22nd 2021 7:09 pm */

#ifndef GAME_MODE_MAIN_COLLISION_H
#define GAME_MODE_MAIN_COLLISION_H
static inline void
UpdateCollisionSub(game_mode_main* Mode,
                   player* Player,
                   list<bullet>* Bullets,
                   assets* Assets,
                   f32 DeltaTime) 
{
    circle2f PlayerCircle = Player->HitCircle;
    v2f PlayerVel = V2f_Sub(Player->Position, Player->PrevPosition);
    PlayerCircle.Origin = V2f_Add(PlayerCircle.Origin, Player->Position);
    
    // Player vs every bullet
    for (u32 I = 0; I < Bullets->Count;) 
    {
        bullet* B = List_Get(Bullets, I);
        circle2f BCircle = B->HitCircle;
        v2f BVel = V2f_Mul(B->Direction, B->Speed * DeltaTime);
        BCircle.Origin = V2f_Add(BCircle.Origin, B->Position);
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          BCircle,
                                          BVel)) 
            
        {
            if (Player->MoodType == B->MoodType) {
                v2f SpawnPos = B->Position;
                SpawnParticleRandomDirectionAndSpeed(Mode,
                                                     Assets,
                                                     SpawnPos,
                                                     5);
                
                List_Slear(Bullets, I);
                continue;
            }
        }
        ++I;
    }
    
}


static inline void
UpdateCollision(game_mode_main* Mode,
                assets* Assets,
                f32 DeltaTime)
{
    UpdateCollisionSub(Mode, &Mode->Player, &Mode->DotBullets, Assets, DeltaTime);
    UpdateCollisionSub(Mode, &Mode->Player, &Mode->CircleBullets, Assets, DeltaTime);
    
}

#endif //GAME_MODE_MAIN_COLLISION_H
