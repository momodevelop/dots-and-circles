/* date = May 22nd 2021 7:10 pm */

#ifndef GAME_MODE_MAIN_PARTICLE_H
#define GAME_MODE_MAIN_PARTICLE_H

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
UpdateParticlesSub(queue<particle>* Q, f32 DeltaTime, u32 Begin, u32 End) {
    for (u32 I = Begin; I <= End; ++I ) {
        particle* P = Q->Data + I;
        P->Timer += DeltaTime;
        
        v2f Velocity = V2f_Mul(P->Direction, P->Speed * DeltaTime);
        P->Position = V2f_Add(P->Position, Velocity);
    }
}


static inline void 
UpdateParticles(game_mode_main* Mode, f32 DeltaTime) {
    queue<particle>* Q = &Mode->Particles;
    if (Queue_IsEmpty(Q)) {
        return;
    }
    // Clean up old particles
    particle * P = Queue_Next(Q);
    while(P != nullptr && P->Timer >= P->Duration) {
        Queue_Pop(Q);
        P = Queue_Next(Q);
    }
    
    // Then update the living ones
    if (Q->Begin <= Q->End) {
        UpdateParticlesSub(Q, DeltaTime, Q->Begin, Q->End);
    }
    else {
        UpdateParticlesSub(Q, DeltaTime, Q->Begin, Q->Cap - 1);
        UpdateParticlesSub(Q, DeltaTime, 0, Q->End);
    }
    
}

static inline void
RenderParticlesSub(queue<particle>* Q, 
                   assets* Assets,
                   mailbox* RenderCommands,
                   u32 Begin, 
                   u32 End, 
                   u32* CurrentCount) 
{
    for (u32 I = Begin; I <= End; ++I ) {
        
        particle* P = Queue_Get(Q, I);
        if (!P) {
            return;
        }
        f32 Ease = 1.f - (P->Timer/P->Duration);
        f32 Alpha = P->Alpha * Ease;
        f32 Size = P->Size * Ease;
        
        f32 Offset = (*CurrentCount)*0.001f;
        
        m44f S = M44f_Scale(Size, Size, 1.f);
        m44f T = M44f_Translation(P->Position.X,
                                  P->Position.Y,
                                  ZLayParticles + Offset);
        
        Draw_TexturedQuadFromImage(RenderCommands,
                                   Assets,
                                   Image_Particle,
                                   M44f_Concat(T,S),
                                   C4f_Create(1.f, 1.f, 1.f, Alpha));
        ++(*CurrentCount);
        
    }
}


static inline void
RenderParticles(game_mode_main* Mode, 
                assets* Assets, 
                mailbox* RenderCommands)

{
    queue<particle>* Q = &Mode->Particles;
    if (Queue_IsEmpty(Q)) {
        return;
    }
    
    // Then update the living ones
    u32 CurrentCount = 0;
    if (Q->Begin <= Q->End) {
        RenderParticlesSub(Q, Assets, RenderCommands, Q->Begin, Q->End, &CurrentCount);
    }
    else {
        RenderParticlesSub(Q, Assets, RenderCommands, Q->Begin, Q->Cap - 1, &CurrentCount);
        RenderParticlesSub(Q, Assets, RenderCommands, 0, Q->End, &CurrentCount);
    }
    
}

#endif //GAME_MODE_MAIN_PARTICLE_H
