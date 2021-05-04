/* date = May 4th 2021 4:50 pm */

#ifndef GAME_AUDIO_MIXER_H
#define GAME_AUDIO_MIXER_H

// TODO(Momo): 
// - A pool of sound counters will reference a sound asset and 'play' it 
// - Output result to buffer, or we can just grab a handle to the game_audio struct
//   and manipulate the audio buffer directly?

struct game_audio_mixer_instance {
    sound_id SoundId; // From Assets
    u32 CurrentOffset; // Current offset of the sound data
    b32 IsLoop;
    b32 IsPlaying;
};

struct game_audio_mixer_handle {
    u32 Id;
};

struct game_audio_mixer {
    game_audio_mixer_instance Instances[32];
    u32 FreeList[ArrayCount(Instances)];
    u32 FreeListCount;
    f32 Volume;
};

static inline void
AudioMixer_Init(game_audio_mixer* Mixer,
                f32 MasterVolume) 
{
    Mixer->Volume = MasterVolume;
    for (u32 I = 0; I < ArrayCount(Mixer->Instances); ++I ){
        Mixer->FreeList[I] = I;
    }
    Mixer->FreeListCount = ArrayCount(Mixer->Instances);
}

static inline game_audio_mixer_handle
AudioMixer_Play(game_audio_mixer* Mixer, 
                sound_id SoundId, 
                b32 Loop) 
{
    u32 Index = Mixer->FreeList[Mixer->FreeListCount - 1];
    game_audio_mixer_instance* Instance = Mixer->Instances + Index;
    Instance->IsLoop = Loop;
    Instance->CurrentOffset = 0;
    Instance->SoundId = SoundId;
    Instance->IsPlaying = True;
    
    --Mixer->FreeListCount;
    
    return { Index };
}


static inline void
AudioMixer_Stop(game_audio_mixer* Mixer,
                game_audio_mixer_handle Handle) 
{
    game_audio_mixer_instance* Instance = Mixer->Instances + Handle.Id;
    Instance->IsPlaying = False;
    Mixer->FreeList[Mixer->FreeListCount++] = Handle.Id;
}

static inline void
AudioMixer_Update(game_audio_mixer* Mixer, 
                  game_audio* Audio,
                  assets* Assets) 
{
    s16* SampleOut = Audio->SampleBuffer;
    for(u32 I = 0; I < Audio->SampleCount; ++I) {
        s16 SampleValue = 0;
        
        for (u32 J = 0; J < ArrayCount(Mixer->Instances); ++J) {
            game_audio_mixer_instance* Instance = Mixer->Instances + J;
            if (Instance->IsPlaying == False) {
                continue;
            }
            sound* Sound = Assets_GetSound(Assets, Instance->SoundId);
            
            SampleValue += s16(Sound->Data[Instance->CurrentOffset++] * 
                               Mixer->Volume);
            
            if (Instance->CurrentOffset >= Sound->DataCount) {
                if (Instance->IsLoop) {
                    Instance->CurrentOffset = 0;
                }
                else {
                    AudioMixer_Stop(Mixer, {J});
                }
            }
            
            
        }
        
        // Based on number of channels!
        *SampleOut++ = SampleValue; // Left Speaker
        *SampleOut++ = SampleValue; // Right Speaker
        
    }
}

#endif //GAME_AUDIO_MIXER_H
