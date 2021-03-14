/* date = March 14th 2021 2:49 pm */

#ifndef MM_WAV_H
#define MM_WAV_H

// NOTE(Momo): http://soundfile.sapp.org/doc/WaveFormat/
struct wav_riff_chunk {
    u32 ID;
    u32 Size;
    u32 Format;
    
};

struct wav_fmt_chunk {
    u32 ID;
    u32 Size;
    u16 AudioFormat;
    u16 NumChannels;
    u32 SampleRate;
    u32 ByteRate;
    u16 BlockAlign;
    u16 BitsPerSample;
};

struct wav_data_chunk {
    u32 ID;
    u32 Size;
    void* Data;
};

struct wav_load_result {
    wav_riff_chunk Riff;
    wav_fmt_chunk Fmt;
    wav_data_chunk Data;
};

static inline b32 
Wav_LoadFromMemory(wav_load_result* Result, 
                   arena* Arena,
                   void* Memory, 
                   u32 MemorySize) 
{
    // TODO(Momo): 
}

#endif //MM_WAV_H
