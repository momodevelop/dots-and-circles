/* date = May 22nd 2021 7:10 pm */

#ifndef GAME_MODE_MAIN_PARTICLE_H
#define GAME_MODE_MAIN_PARTICLE_H

static inline void
SpawnParticle(game_mode_main* Mode, 
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
UpdateParticles(game_mode_main* Mode, f32 DeltaTime) {
    queue<particle>* Q = &Mode->Particles;
    
    auto PopUntilLamb = [](particle* P) {
        return P->Timer >= P->Duration;
    };
    Queue_PopUntil(Q, PopUntilLamb);
    
    
    auto ForEachLamb = [](particle* P, f32 DeltaTime) {
        P->Timer += DeltaTime;
        P->Position += P->Direction * P->Speed * DeltaTime;
    };
    Queue_ForEach(Q, ForEachLamb, DeltaTime);
}


static inline void
RenderParticles(game_mode_main* Mode, 
                assets* Assets, 
                mailbox* RenderCommands)

{
    queue<particle>* Q = &Mode->Particles;
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
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_Particle,
                                   M44f_Concat(T,S),
                                   C4f_Create(1.f, 1.f, 1.f, Alpha));
        ++CurrentCount;
    };
    Queue_ForEach(Q, ForEachLamb);
}

#endif //GAME_MODE_MAIN_PARTICLE_H
