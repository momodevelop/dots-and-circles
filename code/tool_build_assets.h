#ifndef TOOL_BUILD_ASSETS_H
#define TOOL_BUILD_ASSETS_H

#include <stdio.h>
#include <stdlib.h>
#include "ryoji.h"


struct asset_write_context {
    void* DataToWrite;
    usize DataSize;
};

typedef asset_write_context (*asset_builder_write_cb)(void* Context);
typedef void (*asset_builder_free_cb)(asset_write_context WriteContext);


struct asset_entry {
    asset_builder_write_cb WriteCb;
    asset_builder_free_cb FreeCb;
    void* UserContext;
};

template<usize N> 
struct asset_builder {
    asset_entry Entries[N]; // List
    u32 EntryCount;
};

template<usize N> 
static inline void
AddEntry(asset_builder<N>* Builder, 
         void* UserContext, 
         asset_builder_write_cb WriteCb,
         asset_builder_free_cb FreeCb) 
{
    auto* Entry = Builder->Entries + Builder->EntryCount++;
    Entry->UserContext = UserContext;
    Entry->WriteCb = WriteCb;
    Entry->FreeCb = FreeCb;
}


template<usize N> 
static inline b32 
Write(asset_builder<N>* Assets, const char* Filename, const char* Signature) {
    FILE* OutFile = fopen(Filename, "wb");
    if (OutFile == nullptr) {
        return false;
    }
    Defer { fclose(OutFile); };
    
    
    // NOTE(Momo): Write signature
    fwrite(Signature, sizeof(u8), NtsLength(Signature), OutFile);
    
    // NOTE(Momo): Write the amount of items
    u32 EntryCount = Assets->EntryCount;
    fwrite(&EntryCount, sizeof(EntryCount), 1, OutFile);
    
    // NOTE(Momo): Write the data
    for (u32 i = 0; i < EntryCount; ++i) {
        auto* Entry = Assets->Entries + i;
        
        auto WriteContext = Entry->WriteCb(Entry->UserContext);
        fwrite(WriteContext.DataToWrite, WriteContext.DataSize, 1, OutFile);
        Entry->FreeCb(WriteContext);
    }
    
    return true;
}

#endif //TOOL_BUILD_ASSETS_H
