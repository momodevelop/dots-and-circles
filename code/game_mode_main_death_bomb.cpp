
static inline void
Main_UpdateDeathBomb(game_mode_main* Mode, f32 DeltaTime) {
    death_bomb* DeathBomb = &Mode->DeathBomb;
    DeathBomb->Radius += DeathBomb->GrowthSpeed * DeltaTime;
    
    circle2f DeathBombCir = Circle2f_Create(DeathBomb->Position, DeathBomb->Radius);
    
    auto BulletLamb = [&](bullet* B) {
        circle2f BCircle = Circle2f_Offset(B->HitCircle, B->Position);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (Bonk2_IsCircleXCircle(DeathBombCir,
                                  BCircle)) 
        {
            v2f VectorToBullet = V2f_Normalize(B->Position - DeathBomb->Position);
            v2f SpawnPos = DeathBomb->Position + VectorToBullet * DeathBomb->Radius;
            Main_SpawnParticle(Mode, SpawnPos, 5);
            return true;
        }
        
        return false;
    };
    List_ForEachSlearIf(&Mode->CircleBullets, BulletLamb);
    List_ForEachSlearIf(&Mode->DotBullets, BulletLamb);
    
    auto EnemyLamb = [&](enemy* E) {
        circle2f ECir = Circle2f_Create(E->Position, 0.1f);
        
        // NOTE(Momo): We can safely assume that the circles are not moving
        if (Bonk2_IsCircleXCircle(DeathBombCir, ECir)) 
        {
            v2f VectorToBullet = V2f_Normalize(E->Position - DeathBomb->Position);
            v2f SpawnPos = DeathBomb->Position + VectorToBullet * DeathBomb->Radius;
            Main_SpawnParticle(Mode, SpawnPos, 5);
            return true;
        }
        
        return false;
    };
    List_ForEachSlearIf(&Mode->Enemies, EnemyLamb);
}

static inline void
Main_RenderDeathBomb(game_mode_main* Mode)
{
    death_bomb* DeathBomb = &Mode->DeathBomb;
    // Circle?
    Renderer_DrawCircle2f(G_Renderer,
                          Circle2f_Create(DeathBomb->Position, DeathBomb->Radius),
                          5.f, 32, C4f_White, ZLayDeathBomb);
}

