/* date = June 19th 2021 2:28 pm */

#ifndef GAME_PROFILER_H
#define GAME_PROFILER_H


// TODO: Thread this?
struct profiler_entry {
    const char* CustomName;
    const char* FunctionName;
    const char* FileName;
    u32 LineNumber;
    u64 Cycles;
    u32 HitCount;
    b8 IsStarted;
};


profiler_entry G_ProfilerEntries[128];

// NOTE(Momo): Profiler only go up in count;
static inline profiler_entry*
zawarudo_StartProfiling(u32 Index,
                        const char* CustomName,
                        const char* FunctionName,
                        const char* FileName,
                        u32 LineNumber)
{
    
    ASSERT(Index < ARRAY_COUNT(G_ProfilerEntries));
    profiler_entry* E = G_ProfilerEntries + Index;
    ASSERT(!E->IsStarted);
    E->CustomName = CustomName;
    E->FunctionName = FunctionName;
    E->LineNumber = LineNumber;
    E->FileName = FileName;
    E->Cycles -= G_Platform->GetPerformanceCounterFp(); 
    E->IsStarted = true;
    ++E->HitCount;
    
    return E;
}


static inline void
zawarudo_EndProfiling(profiler_entry* E) {
    // TODO: Replace this
    E->Cycles += G_Platform->GetPerformanceCounterFp(); 
    E->IsStarted = false;
}

#define StartProfiling(Name) profiler_entry* Name = zawarudo_StartProfiling(__COUNTER__, #Name, __FUNCTION__, __FILE__, __LINE__);
#define EndProfiling(Name) zawarudo_EndProfiling(Name);

#endif //GAME_PROFILER_H
