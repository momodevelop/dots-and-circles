/* date = May 4th 2021 4:50 pm */

#ifndef GAME_AUDIO_MIXER_H
#define GAME_AUDIO_MIXER_H

// TODO(Momo): 
// - A pool of sound counters will reference a sound asset and 'play' it 
// - Output result to buffer, or we can just grab a handle to the game_audio struct
//   and manipulate the audio buffer directly?

struct AudioMixer_Instance {
    Sound_ID sound_id; // From Assets
    u32 current_offset; // Current offset of the sound data
    b8 is_loop;
    b8 is_playing;
};


// NOTE(Momo): id == 0 is invalid handle.
// IDs are +1 of the actual indices
struct AudioMixer_Handle {
    u32 id;
    b8 is_valid;
    inline operator bool() {
        return id != 0; 
    }
    
};

struct AudioMixer {
    Array<AudioMixer_Instance> instances;
    List<u32> free_list;
    f32 volume;
};

static inline b8
AudioMixer_Init(AudioMixer* a,
                f32 master_volume,
                u32 max_instances,
                Arena* arena) 
{
    // NOTE(Momo): We reject the highest value because
    // we are reserving one index to represent an invalid index
    // which is index == 0
    if (max_instances == 0 || max_instances == U32_MAX) {
        return false;
    }
    Arena_Marker mark = Arena_Mark(arena);
    b8 success = Array_Alloc(&a->instances, arena, max_instances);
    if (!success) {
        Arena_Revert(&mark);
        return false;
    }
    
    success = List_Alloc(&a->free_list, arena, max_instances);
    if (!success) {
        Arena_Revert(&mark);
        return false;
    }
    a->volume = master_volume;
    
    for (u32 i = 0; i < a->instances.count; ++i ){
        a->instances[i] = {};
        u32* item = List_Push(&a->free_list);
        if(!item) {
            return false;
        }
        (*item) = i;
    }
    
    return true;
}

static inline AudioMixer_Handle
AudioMixer_Play(AudioMixer* a, 
                Sound_ID sound_id, 
                b8 loop)
{
    AudioMixer_Handle ret = {};
    u32* index_ptr = List_Last(&a->free_list);
    if (index_ptr == nullptr) {
        return ret;
    }
    List_Pop(&a->free_list);
    u32 index = (*index_ptr);
    
    AudioMixer_Instance* instance = a->instances + index;
    instance->is_loop = loop;
    instance->current_offset = 0;
    instance->sound_id = sound_id;
    instance->is_playing = true;
    
    ret.id = index;
    
    return ret;
}


static inline b8
AudioMixer_Stop(AudioMixer* a,AudioMixer_Handle handle) 
{
    if(!handle) {
        return false;
    }
    
    if ((u32)handle.id < a->instances.count) {
        AudioMixer_Instance* instance = a->instances + handle.id;
        instance->is_playing = false;
        u32* item = List_Push(&a->free_list);
        if (item == nullptr) {
            return false;
        }
        (*item) = handle.id;
        return true;
    }
    return false;
}


static inline void
AudioMixer_Update(AudioMixer* a,Platform_Audio* audio) 
{
    s16* sample_out = audio->sample_buffer;
    for(u32 I = 0; I < audio->sample_count; ++I) {
        
        for (u32 J = 0; J < audio->channels; ++J) {
            sample_out[J] = 0;
        }
        
        for (u32 J = 0; J < a->instances.count; ++J) {
            AudioMixer_Instance* instance = a->instances + J;
            if (instance->is_playing == false) {
                continue;
            }
            Sound* sound = Assets_GetSound(g_assets, instance->sound_id);
            
            for (u32 K = 0; K < audio->channels; ++K) {
                sample_out[K] += s16(sound->data[instance->current_offset++] * 
                                     a->volume);
            }
            
            if (instance->current_offset >= sound->data_count) {
                if (instance->is_loop) {
                    instance->current_offset = 0;
                }
                else {
                    AudioMixer_Stop(a, {J});
                }
            }
            
            
            
        }
        sample_out += audio->channels;
        
    }
}


#endif //GAME_AUDIO_MIXER_H
