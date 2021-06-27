
static inline void
Main_UpdateDeathBomb(game_mode_main* Mode, f32 DeltaTime) {
    death_bomb* DeathBomb = &Mode->DeathBomb;
    DeathBomb->Radius += DeathBomb->GrowthSpeed * DeltaTime;
    
    circle2f DeathBombCir = circle2f::create(DeathBomb->Position, DeathBomb->Radius);
    
    auto BulletLamb = [&](bullet* B) {
        circle2f BCircle = translate(B->HitCircle, B->Position);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (Bonk2_IsCircleXCircle(DeathBombCir,
                                  BCircle)) 
        {
            v2f VectorToBullet = normalize(B->Position - DeathBomb->Position);
            v2f SpawnPos = DeathBomb->Position + VectorToBullet * DeathBomb->Radius;
            Main_SpawnParticle(Mode, SpawnPos, 5);
            return true;
        }
        
        return false;
    };
    Mode->CircleBullets.foreach_slear_if(BulletLamb);
    Mode->DotBullets.foreach_slear_if(BulletLamb);
    
    auto EnemyLamb = [&](enemy* E) {
        circle2f ECir = circle2f::create(E->Position, 0.1f);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (Bonk2_IsCircleXCircle(DeathBombCir, ECir)) 
        {
            v2f VectorToBullet = normalize(E->Position - DeathBomb->Position);
            v2f SpawnPos = DeathBomb->Position + VectorToBullet * DeathBomb->Radius;
            Main_SpawnParticle(Mode, SpawnPos, 5);
            return true;
        }
        
        return false;
    };
    Mode->Enemies.foreach_slear_if(EnemyLamb);
}

static inline void
Main_RenderDeathBomb(game_mode_main* Mode)
{
    death_bomb* DeathBomb = &Mode->DeathBomb;
    // Circle?
    Renderer_DrawCircle2f(G_Renderer,
                          circle2f::create(DeathBomb->Position, DeathBomb->Radius),
                          5.f, 32, C4F_WHITE, ZLayDeathBomb);
}

