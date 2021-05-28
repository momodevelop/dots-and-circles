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
    PlayerCircle.Origin += Player->Position;
    
    auto Lamb = [&](bullet* B) {
        circle2f BCircle = B->HitCircle;
        v2f BVel = B->Direction * B->Speed * DeltaTime;
        BCircle.Origin += B->Position;
        
        if (Bonk2_IsDynaCircleXDynaCircle(PlayerCircle, 
                                          PlayerVel,
                                          BCircle,
                                          BVel)) 
        {
            if (Player->MoodType == B->MoodType) {
                v2f VectorToBullet = V2f_Normalize(B->Position - Player->Position);
                v2f SpawnPos = Player->Position + VectorToBullet * Player->HitCircle.Radius;
                Main_SpawnParticle(Mode, SpawnPos, 5);
            }
            else {
                Player->IsDead = true;
            }
            return true;
        }
        
        return false;
    };
    List_ForEachSlearIf(&Mode->DotBullets, Lamb);
    List_ForEachSlearIf(&Mode->CircleBullets, Lamb);
}

#endif //GAME_MODE_MAIN_COLLISION_H
