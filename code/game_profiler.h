/* date = June 19th 2021 2:28 pm */

#ifndef GAME_PROFILER_H
#define GAME_PROFILER_H


// TODO: Thread this?
struct Profiler_Entry {
    const char* custom_name;
    const char* function_name;
    const char* file_name;
    u32 line_number;
    u64 cycles;
    u32 hit_count;
    b8 is_started;
};


Profiler_Entry G_ProfilerEntries[128];

// NOTE(Momo): Profiler only go up in count;
static inline Profiler_Entry*
zawarudo_start_profiling(u32 index,
                         const char* custom_name,
                         const char* function_name,
                         const char* file_name,
                         u32 line_number)
{
    
    ASSERT(index < ArrayCount(G_ProfilerEntries));
    Profiler_Entry* e = G_ProfilerEntries + index;
    ASSERT(!e->is_started);
    e->custom_name = custom_name;
    e->function_name = function_name;
    e->line_number = line_number;
    e->file_name = file_name;
    e->cycles -= g_platform->get_performance_counter(); 
    e->is_started = true;
    ++e->hit_count;
    
    return e;
}


static inline void
zawarudo_end_profiling(Profiler_Entry* E) {
    // TODO: Replace this
    E->cycles += g_platform->get_performance_counter(); 
    E->is_started = false;
}

#define START_PROFILING(Name) profiler_entry* Name = zawarudo_start_profiling(__COUNTER__, #Name, __FUNCTION__, __FILE__, __LINE__);
#define END_PROFILING(Name) zawarudo_end_profiling(Name);

#endif //GAME_PROFILER_H
