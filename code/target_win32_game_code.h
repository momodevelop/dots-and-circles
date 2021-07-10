/* date = July 10th 2021 3:23 pm */

#ifndef TARGET_WIN32_GAME_CODE_H
#define TARGET_WIN32_GAME_CODE_H
struct Win32_Game_Code {
    HMODULE dll;
    Game_Update* game_update;
    FILETIME prev_write_time;
    
    const char *src_filename;
    const char *temp_filename;
    const char *lock_filename;
    
    
    void init(const char* src_filename,
              const char* temp_filename,
              const char* lock_filename);
    b8 load();
    void unload();
    b8 is_outdated();
};


void
Win32_Game_Code::init(const char* src_filename_in,
                      const char* temp_filename_in,
                      const char* lock_filename_in) 
{
    
    
    this->src_filename = src_filename_in;
    this->temp_filename = temp_filename_in;
    this->lock_filename = lock_filename_in;
}

b8
Win32_Game_Code::load() 
{
    WIN32_FILE_ATTRIBUTE_DATA ignored; 
    if(!GetFileAttributesEx(this->lock_filename, 
                            GetFileExInfoStandard, 
                            &ignored)) 
    {
        this->prev_write_time = win32_get_file_last_write_time(this->src_filename);
        BOOL success = FALSE;
        do {
            success = CopyFile(this->src_filename, this->temp_filename, FALSE); 
        } while (!success); 
        this->dll = LoadLibraryA(this->temp_filename);
        if(this->dll) {
            this->game_update = 
                (Game_Update*)GetProcAddress(this->dll, "game_update");
            if(this->game_update == 0) {
                return false;
            }
            return true;
        }
        return false;
        
    }
    return false;
}

void 
Win32_Game_Code::unload() {
    if (this->dll) {
        FreeLibrary(this->dll);
        this->dll = 0;
    }
    this->game_update = 0;
}

b8
Win32_Game_Code::is_outdated() {    
    // Check last modified date
    FILETIME last_write_time = win32_get_file_last_write_time(this->src_filename);
    LARGE_INTEGER current_last_write_time = win32_file_time_to_large_int(last_write_time); 
    LARGE_INTEGER game_code_last_write_time = win32_file_time_to_large_int(this->prev_write_time);
    
    return (current_last_write_time.QuadPart > game_code_last_write_time.QuadPart); 
}

#endif //TARGET_WIN32_GAME_CODE_H
