/* date = March 14th 2021 2:49 pm */
// TODO: we might want to refactor some of the code here
#ifndef MM_WAV_H
#define MM_WAV_H

#define WAV_RIFF_ID_SIGNATURE 0x52494646
#define WAV_RIFF_FORMAT_SIGNATURE 0x57415645
#define WAV_FMT_ID_SIGNATURE 0x666d7420
#define WAV_DATA_ID_SIGNATURE 0x64617461

// NOTE(Momo): http://soundfile.sapp.org/doc/Waveformat/
struct Wav_Riff_Chunk {
    u32 id; // big endian
    u32 size;
    u32 format; // big endian
    
};



struct Wav_Fmt_Chunk {
    u32 id;
    u32 size;
    u16 audio_format;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
};

struct Wav_Data_Chunk {
    u32 id;
    u32 size;
};

struct Wav_Load_Result {
    Wav_Riff_Chunk riff_chunk;
    Wav_Fmt_Chunk fmt_chunk;
    Wav_Data_Chunk data_chunk;
    void* data;
};

// NOTE(Momo): Will actually leave data into arena
static inline b8 
load_wav_from_memory(Wav_Load_Result* result,
                     void* memory, 
                     u32 memory_size) 
{
    Stream stream = {};
    if (!Stream_Init(&stream, memory, memory_size)) {
        return false;
    }
    
    // NOTE(Momo): Load Riff Chunk
    auto* riff_chunk = Stream_Consume<Wav_Riff_Chunk>(&stream);
    if (!riff_chunk) {
        return false;
    }
    EndianSwap(&riff_chunk->id);
    EndianSwap(&riff_chunk->format);
    if (riff_chunk->id != WAV_RIFF_ID_SIGNATURE) {
        return false;
    }
    if (riff_chunk->format != WAV_RIFF_FORMAT_SIGNATURE) {
        return false;
    }
    
    // NOTE(Momo): Load fmt Chunk
    auto* fmt_chunk = Stream_Consume<Wav_Fmt_Chunk>(&stream);
    if (!fmt_chunk) {
        return false;
    }
    EndianSwap(&fmt_chunk->id);
    if (fmt_chunk->id != WAV_FMT_ID_SIGNATURE) {
        return false;
    }
    if (fmt_chunk->size != 16) {
        return false;
    }
    if (fmt_chunk->audio_format != 1) {
        return false;
    }
    
    u32 BytesPerSample = fmt_chunk->bits_per_sample/8;
    if (fmt_chunk->byte_rate != 
        fmt_chunk->sample_rate * fmt_chunk->num_channels * BytesPerSample) {
        return false;
    }
    if (fmt_chunk->block_align != fmt_chunk->num_channels * BytesPerSample) {
        return false;
    }
    
    // NOTE(Momo): Load data Chunk
    auto* data_chunk = Stream_Consume<Wav_Data_Chunk>(&stream);
    if (!data_chunk) {
        return false;
    }
    EndianSwap(&data_chunk->id);
    if (data_chunk->id != WAV_DATA_ID_SIGNATURE) {
        return false;
    }
    
    void* data = Stream_Consume_Block(&stream, data_chunk->size);
    if (data == nullptr) {
        return false;
    }
    
    result->riff_chunk = (*riff_chunk);
    result->fmt_chunk = (*fmt_chunk);
    result->data_chunk = (*data_chunk);
    result->data = data;
    
    return true;
}

#endif //MM_WAV_H
