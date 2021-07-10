/* date = July 10th 2021 0:23 am */

#ifndef TARGET_WIN32_GAME_MEMORY_H
#define TARGET_WIN32_GAME_MEMORY_H

struct Win32_Game_Memory {
    Game_Memory head;
    
    void* data;
    u32 data_size;
    
    b8 init(u32 permanent_memory_size,
            u32 transient_memory_size,
            u32 scratch_memory_size,
            u32 debug_memory_size);
    void free();
    b8 save(const char* path);
    b8 load(const char* path);
};

void
Win32_Game_Memory::free() {
    win32_log("[Win32::game_memory] Freed\n");
    win32_free_memory(this->data);
}

b8
Win32_Game_Memory::init(u32 permanent_memory_size,
                        u32 transient_memory_size,
                        u32 scratch_memory_size,
                        u32 debug_memory_size) 
{
    this->data_size = permanent_memory_size + transient_memory_size + scratch_memory_size + debug_memory_size;
    
#if INTERNAL
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);
    
    this->data = 
        win32_allocate_memory_at_address(this->data_size, 
                                         SystemInfo.lpMinimumApplicationAddress);
#else
    this->data = win32_allocate_memory(this->data_size);
#endif
    if (!this->data) {
        return false;
    }
    
    u8* memory_ptr = (u8*)this->data;
    
    this->head.permanent_memory_size = permanent_memory_size;
    this->head.permanent_memory = this->data;
    memory_ptr += permanent_memory_size;
    
    this->head.transient_memory_size = transient_memory_size;
    this->head.transient_memory = memory_ptr;
    memory_ptr += transient_memory_size;
    
    this->head.scratch_memory_size = scratch_memory_size;
    this->head.scratch_memory = memory_ptr;
    memory_ptr += scratch_memory_size;
    
    this->head.debug_memory_size = debug_memory_size;
    this->head.debug_memory = memory_ptr;
    
    return true;
}

b8
Win32_Game_Memory::save(const char* path) {
    // We just dump the whole game memory into a file
    HANDLE win32_handle = CreateFileA(path,
                                      GENERIC_WRITE,
                                      FILE_SHARE_WRITE,
                                      0,
                                      CREATE_ALWAYS,
                                      0,
                                      0);
    if (win32_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    defer { CloseHandle(win32_handle); }; 
    
    DWORD bytes_written;
    if(!WriteFile(win32_handle, 
                  this->data,
                  (DWORD)this->data_size,
                  &bytes_written,
                  0)) 
    {
        return false;
    }
    
    if (bytes_written != this->data_size) {
        return false;
    }
    return true; 
    
}

b8
Win32_Game_Memory::load(const char* path) {
    HANDLE win32_handle = CreateFileA(path,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      0,
                                      OPEN_EXISTING,
                                      0,
                                      0);
    if (win32_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    defer { CloseHandle(win32_handle); }; 
    DWORD bytes_read;
    
    BOOL success = ReadFile(win32_handle, 
                            this->data,
                            (DWORD)this->data_size,
                            &bytes_read,
                            0);
    
    if (success && this->data_size == bytes_read) {
        return false;
    }
    
    return true;
}



#endif //TARGET_WIN32_GAME_MEMORY_H
