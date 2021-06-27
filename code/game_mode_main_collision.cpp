/* date = May 22nd 2021 7:09 pm */

#ifndef GAME_MODE_MAIN_COLLISION_H
#define GAME_MODE_MAIN_COLLISION_H


// TODO: We could put this into Bullet update
static inline void
Main_UpdatePlayerBulletCollision(game_mode_main* Mode,
                                 assets* Assets,
                                 f32 DeltaTime)
{
    player* Player = &Mode->Player;
    circle2f PlayerCircle = Player->HitCircle;
    v2f PlayerVel = Player->Position - Player->PrevPosition;
    PlayerCircle.origin += Player->Position;
    
    auto Lamb = [&](bullet* B) {
        circle2f BCircle = B->HitCircle;
        v2f BVel = B->Direction * B->Speed * DeltaTime;
        BCircle.origin += B->Position;
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          BCircle,
                                          BVel)) 
        {
            if (Player->MoodType == B->MoodType) {
                v2f VectorToBullet = normalize(B->Position - Player->Position);
                v2f SpawnPos = Player->Position + VectorToBullet * Player->HitCircle.radius;
                Main_SpawnParticle(Mode, SpawnPos, 5);
                Mode->Score += 50;
            }
            else {
                Player->IsDead = true;
            }
            return true;
        }
        
        return false;
    };
    Mode->DotBullets.foreach_slear_if(Lamb);
    Mode->CircleBullets.foreach_slear_if(Lamb);
}

#endif //GAME_MODE_MAIN_COLLISION_H
