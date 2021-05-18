/* date = March 14th 2021 2:49 pm */

#ifndef MM_WAV_H
#define MM_WAV_H


#define Wav_RiffIDSignature 0x52494646
#define Wav_RiffFormatSignature 0x57415645
#define Wav_FmtIDSignature 0x666d7420
#define Wav_DataIDSignature 0x64617461

// NOTE(Momo): http://soundfile.sapp.org/doc/WaveFormat/
struct wav_riff_chunk {
    u32 ID; // big endian
    u32 Size;
    u32 Format; // big endian
    
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
};

struct wav_load_result {
    wav_riff_chunk RiffChunk;
    wav_fmt_chunk FmtChunk;
    wav_data_chunk DataChunk;
    void* Data;
};

// NOTE(Momo): Will actually leave data into arena
static inline b32 
Wav_LoadFromMemory(wav_load_result* Result,
                   void* Memory, 
                   u32 MemorySize) 
{
    stream Stream = Stream_CreateFromMemory(Memory, MemorySize);
    
    // NOTE(Momo): Load Riff Chunk
    auto* RiffChunk = Stream_Consume<wav_riff_chunk>(&Stream);
    if (!RiffChunk) {
        return False;
    }
    EndianSwapU32(&RiffChunk->ID);
    EndianSwapU32(&RiffChunk->Format);
    if (RiffChunk->ID != Wav_RiffIDSignature) {
        return False;
    }
    if (RiffChunk->Format != Wav_RiffFormatSignature) {
        return False;
    }
    
    // NOTE(Momo): Load fmt Chunk
    auto* FmtChunk = Stream_Consume<wav_fmt_chunk>(&Stream);
    if (!FmtChunk) {
        return False;
    }
    EndianSwapU32(&FmtChunk->ID);
    if (FmtChunk->ID != Wav_FmtIDSignature) {
        return False;
    }
    if (FmtChunk->Size != 16) {
        return False;
    }
    if (FmtChunk->AudioFormat != 1) {
        return False;
    }
    
    u32 BytesPerSample = FmtChunk->BitsPerSample/8;
    if (FmtChunk->ByteRate != 
        FmtChunk->SampleRate * FmtChunk->NumChannels * BytesPerSample) {
        return False;
    }
    if (FmtChunk->BlockAlign != FmtChunk->NumChannels * BytesPerSample) {
        return False;
    }
    
    // NOTE(Momo): Load data Chunk
    auto* DataChunk = Stream_Consume<wav_data_chunk>(&Stream);
    if (!DataChunk) {
        return False;
    }
    EndianSwapU32(&DataChunk->ID);
    if (DataChunk->ID != Wav_DataIDSignature) {
        return False;
    }
    
    void* Data = Stream_ConsumeBlock(&Stream, DataChunk->Size);
    if (Data == Null) {
        return False;
    }
    
    Result->RiffChunk = (*RiffChunk);
    Result->FmtChunk = (*FmtChunk);
    Result->DataChunk = (*DataChunk);
    Result->Data = Data;
    
    return True;
}

#endif //MM_WAV_H
