/* date = May 4th 2021 4:50 pm */

#ifndef GAME_AUDIO_MIXER_H
#define GAME_AUDIO_MIXER_H

// TODO(Momo): 
// - A pool of sound counters will reference a sound asset and 'play' it 
// - Output result to buffer, or we can just grab a handle to the game_audio struct
//   and manipulate the audio buffer directly?

struct Game_Audio_Mixer_Instance {
    Sound_ID sound_id; // From Assets
    u32 current_offset; // Current offset of the sound data
    b8 is_loop;
    b8 is_playing;
};


// NOTE(Momo): id == 0 is invalid handle.
// IDs are +1 of the actual indices
struct Game_Audio_Mixer_Handle {
    u32 id;
    b8 is_valid;
    inline operator bool();
};

struct Game_Audio_Mixer {
    Array<Game_Audio_Mixer_Instance> instances;
    List<u32> free_list;
    f32 volume;
    
    b8 init(f32 master_volume, u32 max_instances, Arena* arena);
    Game_Audio_Mixer_Handle play(Sound_ID sound_id, b8 loop);
    b8 stop(Game_Audio_Mixer_Handle handle);
    void update(platform_audio* audio);
};

b8
Game_Audio_Mixer::init(f32 master_volume,
                       u32 max_instances,
                       Arena* arena) 
{
    // NOTE(Momo): We reject the highest value because
    // we are reserving one index to represent an invalid index
    // which is index == 0
    if (max_instances == 0 || max_instances == U32_MAX) {
        return false;
    }
    Arena_Mark mark = arena->mark();
    b8 success = this->instances.alloc(arena, max_instances);
    if (!success) {
        mark.revert();
        return false;
    }
    
    success = this->free_list.alloc(arena, max_instances);
    if (!success) {
        mark.revert();
        return false;
    }
    this->volume = master_volume;
    
    for (u32 i = 0; i < this->instances.count; ++i ){
        this->instances[i] = {};
        u32* item = this->free_list.push();
        if(!item) {
            return false;
        }
        (*item) = i;
    }
    
    return true;
}

Game_Audio_Mixer_Handle
Game_Audio_Mixer::play(Sound_ID sound_id, 
                       b8 loop)
{
    Game_Audio_Mixer_Handle ret = {};
    u32* index_ptr = this->free_list.last();
    if (index_ptr == nullptr) {
        return ret;
    }
    this->free_list.pop();
    u32 index = (*index_ptr);
    
    Game_Audio_Mixer_Instance* instance = this->instances + index;
    instance->is_loop = loop;
    instance->current_offset = 0;
    instance->sound_id = sound_id;
    instance->is_playing = true;
    
    ret.id = index;
    
    return ret;
}


b8
Game_Audio_Mixer::stop(Game_Audio_Mixer_Handle handle) 
{
    if(!handle) {
        return false;
    }
    
    if ((u32)handle.id < this->instances.count) {
        Game_Audio_Mixer_Instance* instance = this->instances + handle.id;
        instance->is_playing = false;
        u32* item = this->free_list.push();
        if (item == nullptr) {
            return false;
        }
        (*item) = handle.id;
        return true;
    }
    return false;
}


void
Game_Audio_Mixer::update(platform_audio* audio) 
{
    s16* sample_out = audio->SampleBuffer;
    for(u32 I = 0; I < audio->SampleCount; ++I) {
        
        for (u32 J = 0; J < audio->Channels; ++J) {
            sample_out[J] = 0;
        }
        
        for (u32 J = 0; J < this->instances.count; ++J) {
            Game_Audio_Mixer_Instance* instance = this->instances + J;
            if (instance->is_playing == false) {
                continue;
            }
            Sound* sound = G_Assets->get_sound(instance->sound_id);
            
            for (u32 K = 0; K < audio->Channels; ++K) {
                sample_out[K] += s16(sound->data[instance->current_offset++] * 
                                     this->volume);
            }
            
            if (instance->current_offset >= sound->data_count) {
                if (instance->is_loop) {
                    instance->current_offset = 0;
                }
                else {
                    stop({J});
                }
            }
            
            
            
        }
        sample_out += audio->Channels;
        
    }
}


inline
Game_Audio_Mixer_Handle::operator bool() 
{
    return id != 0; 
}

#endif //GAME_AUDIO_MIXER_H
