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
    MM_Array<game_audio_mixer_instance> Instances;
    MM_List<u32> FreeList;
    
    f32 Volume;
};

static inline b32
AudioMixer_Init(game_audio_mixer* Mixer,
                f32 MasterVolume,
                u32 MaxInstances,
                MM_Arena* Arena) 
{
    MM_ArenaMark Mark = MM_Arena_Mark(Arena);
    b32 Success = MM_Array_New(&Mixer->Instances, Arena, MaxInstances);
    if (!Success) {
        MM_Arena_Revert(&Mark);
        return False;
    }
    
    Success = MM_List_New(&Mixer->FreeList, Arena, MaxInstances);
    if (!Success) {
        MM_Arena_Revert(&Mark);
        return False;
    }
    Mixer->Volume = MasterVolume;
    
    for (u32 I = 0; I < Mixer->Instances.count; ++I ){
        Mixer->Instances[I] = {};
        u32* Item = MM_List_Push(&Mixer->FreeList);
        Assert(Item);
        (*Item) = I;
    }
    
    return True;
}

static inline b32
AudioMixer_Play(game_audio_mixer* Mixer, 
                sound_id SoundId, 
                b32 Loop,
                game_audio_mixer_handle* OutHandle)
{
    u32* Index = MM_List_Last(&Mixer->FreeList);
    if (Index == Null) {
        return False;
    }
    
    game_audio_mixer_instance* Instance = Mixer->Instances + (*Index);
    Instance->IsLoop = Loop;
    Instance->CurrentOffset = 0;
    Instance->SoundId = SoundId;
    Instance->IsPlaying = True;
    
    MM_List_Pop(&Mixer->FreeList);
    
    
    OutHandle->Id = (*Index);
    
    return True;
}


static inline b32
AudioMixer_Stop(game_audio_mixer* Mixer,
                game_audio_mixer_handle Handle) 
{
    if (Handle.Id < Mixer->Instances.count) {
        game_audio_mixer_instance* Instance = Mixer->Instances + Handle.Id;
        Instance->IsPlaying = False;
        u32* Item = MM_List_Push(&Mixer->FreeList);
        if (Item == Null) {
            return False;
        }
        (*Item) = Handle.Id;
        return True;
    }
    return False;
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
        
        for (u32 J = 0; J < Mixer->Instances.count; ++J) {
            game_audio_mixer_instance* Instance = Mixer->Instances + J;
            if (Instance->IsPlaying == False) {
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
