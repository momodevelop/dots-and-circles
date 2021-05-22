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
    b8 IsLoop;
    b8 IsPlaying;
};

struct game_audio_mixer_handle {
    u32 Id;
};

struct game_audio_mixer {
    array<game_audio_mixer_instance> Instances;
    list<u32> FreeList;
    
    f32 Volume;
};

static inline b8
AudioMixer_Init(game_audio_mixer* Mixer,
                f32 MasterVolume,
                u32 MaxInstances,
                arena* Arena) 
{
    arena_mark Mark = Arena_Mark(Arena);
    b8 Success = Array_New(&Mixer->Instances, Arena, MaxInstances);
    if (!Success) {
        Arena_Revert(&Mark);
        return false;
    }
    
    Success = List_New(&Mixer->FreeList, Arena, MaxInstances);
    if (!Success) {
        Arena_Revert(&Mark);
        return false;
    }
    Mixer->Volume = MasterVolume;
    
    for (u32 I = 0; I < Mixer->Instances.Count; ++I ){
        Mixer->Instances[I] = {};
        u32* Item = List_Push(&Mixer->FreeList);
        Assert(Item);
        (*Item) = I;
    }
    
    return true;
}

static inline b8
AudioMixer_Play(game_audio_mixer* Mixer, 
                sound_id SoundId, 
                b8 Loop,
                game_audio_mixer_handle* OutHandle)
{
    u32* Index = List_Last(&Mixer->FreeList);
    if (Index == nullptr) {
        return false;
    }
    
    game_audio_mixer_instance* Instance = Mixer->Instances + (*Index);
    Instance->IsLoop = Loop;
    Instance->CurrentOffset = 0;
    Instance->SoundId = SoundId;
    Instance->IsPlaying = true;
    
    List_Pop(&Mixer->FreeList);
    
    
    OutHandle->Id = (*Index);
    
    return true;
}


static inline b8
AudioMixer_Stop(game_audio_mixer* Mixer,
                game_audio_mixer_handle Handle) 
{
    if (Handle.Id < Mixer->Instances.Count) {
        game_audio_mixer_instance* Instance = Mixer->Instances + Handle.Id;
        Instance->IsPlaying = false;
        u32* Item = List_Push(&Mixer->FreeList);
        if (Item == nullptr) {
            return false;
        }
        (*Item) = Handle.Id;
        return true;
    }
    return false;
}


static inline void
AudioMixer_Update(game_audio_mixer* Mixer, 
                  platform_audio* Audio,
                  assets* Assets) 
{
    s16* SampleOut = Audio->SampleBuffer;
    for(u32 I = 0; I < Audio->SampleCount; ++I) {
        
        for (u32 J = 0; J < Audio->Channels; ++J) {
            SampleOut[J] = 0;
        }
        
        for (u32 J = 0; J < Mixer->Instances.Count; ++J) {
            game_audio_mixer_instance* Instance = Mixer->Instances + J;
            if (Instance->IsPlaying == false) {
                continue;
            }
            sound* Sound = Assets_GetSound(Assets, Instance->SoundId);
            
            for (u32 K = 0; K < Audio->Channels; ++K) {
                SampleOut[K] += s16(Sound->Data[Instance->CurrentOffset++] * 
                                    Mixer->Volume);
            }
            
            if (Instance->CurrentOffset >= Sound->DataCount) {
                if (Instance->IsLoop) {
                    Instance->CurrentOffset = 0;
                }
                else {
                    AudioMixer_Stop(Mixer, {J});
                }
            }
            
            
        }
        SampleOut += Audio->Channels;
        
    }
}

#endif //GAME_AUDIO_MIXER_H
