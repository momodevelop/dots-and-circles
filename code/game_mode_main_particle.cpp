static inline void
Main_SpawnParticle(game_mode_main* Mode, 
                   v2f Position,
                   u32 Amount) 
{
    for (u32 I = 0; I < Amount; ++I) {
        particle* P = Mode->Particles.push();
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
Main_UpdateParticles(game_mode_main* Mode, f32 DeltaTime) {
    Queue<particle>* Q = &Mode->Particles;
    
    auto PopUntilLamb = [](particle* P) {
        return P->Timer >= P->Duration;
    };
    Q->pop_until(PopUntilLamb);
    
    
    auto ForEachLamb = [](particle* P, f32 DeltaTime) {
        P->Timer += DeltaTime;
        P->Position += P->Direction * P->Speed * DeltaTime;
    };
    Q->foreach(ForEachLamb, DeltaTime);
}


static inline void
Main_RenderParticles(game_mode_main* Mode) {
    Queue<particle>* Q = &Mode->Particles;
    u32 CurrentCount = 0;
    auto ForEachLamb = [&](particle* P) {
        f32 Ease = 1.f - (P->Timer/P->Duration);
        f32 Alpha = P->Alpha * Ease;
        f32 Size = P->Size * Ease;
        
        f32 Offset = CurrentCount*0.001f;
        
        m44f S = M44f_Scale(Size, Size, 1.f);
        m44f T = M44f_Translation(P->Position.X,
                                  P->Position.Y,
                                  ZLayParticles + Offset);
        
        Draw_TexturedQuadFromImage(Image_Particle,
                                   M44f_Concat(T,S),
                                   C4f_Create(1.f, 1.f, 1.f, Alpha));
        ++CurrentCount;
    };
    Q->foreach(ForEachLamb);
}
