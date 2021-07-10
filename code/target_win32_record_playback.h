/* date = July 10th 2021 0:49 pm */

#ifndef TARGET_WIN32_RECORD_PLAYBACK_H
#define TARGET_WIN32_RECORD_PLAYBACK_H

struct Win32_Input_Recorder {
    b8 is_recording_input;
    HANDLE recording_input_handle;
    
    b8 begin(const char* path);
    b8 update(Platform_Input* input);
    b8 end();
    
};


struct Win32_Input_Playbacker {
    b8 is_playback_input;
    HANDLE playback_input_handle;
    
    b8 begin(const char* path);
    b8 update(Platform_Input* input);
    b8 end();
    
};

b8
Win32_Input_Recorder::begin(const char* path) {
    if(this->is_recording_input){
        return false;
    }
    HANDLE record_file_handle = CreateFileA(path,
                                            GENERIC_WRITE,
                                            FILE_SHARE_WRITE,
                                            0,
                                            CREATE_ALWAYS,
                                            0,
                                            0);
    
    if (record_file_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::BeginRecordingInput] Cannot open file: %s\n", path);
        return false;
    }
    this->recording_input_handle = record_file_handle;
    this->is_recording_input = true;
    win32_log("[Win32::BeginRecordingInput] Recording has begun: %s\n", path);
    return true;
}

b8
Win32_Input_Recorder::end() {
    if (!this->is_recording_input) {
        return false;
    }
    CloseHandle(this->recording_input_handle);
    this->is_recording_input = false;
    
    return true;
}

b8
Win32_Input_Recorder::update(Platform_Input* input) {
    if(!this->is_recording_input) {
        return false;
    }
    DWORD BytesWritten;
    if(!WriteFile(this->recording_input_handle,
                  input,
                  sizeof(Platform_Input),
                  &BytesWritten, 0)) 
    {
        win32_log("[Win32::Recordinput] Cannot write file\n");
        this->end();
        return false;
    }
    
    if (BytesWritten != sizeof(Platform_Input)) {
        win32_log("[Win32::Recordinput] Did not complete writing\n");
        this->end();
        return false;
    }
    return true;
}

b8
Win32_Input_Playbacker::end() {
    if(!this->is_playback_input) {
        return false;
    }
    CloseHandle(this->playback_input_handle);
    this->is_playback_input = false;
    win32_log("[Win32::EndPlaybackInput] Playback has ended\n");
    return true;
}

b8
Win32_Input_Playbacker::begin(const char* path) {
    if(this->is_playback_input){
        return false;
    }
    HANDLE record_file_handle = CreateFileA(path,
                                            GENERIC_READ,
                                            FILE_SHARE_READ,
                                            0,
                                            OPEN_EXISTING,
                                            0,
                                            0);
    
    if (record_file_handle == INVALID_HANDLE_VALUE) {
        win32_log("[Win32::BeginPlaybackInput] Cannot open file: %s\n", path);
        return false;
    }
    this->playback_input_handle = record_file_handle;
    this->is_playback_input = true;
    win32_log("[Win32::BeginPlaybackInput] Playback has begun: %s\n", path);
    return true;
}

// NOTE(Momo): returns true if 'done' reading all input, false otherwise
b8 
Win32_Input_Playbacker::update(Platform_Input* input) {
    DWORD bytes_read;
    BOOL success = ReadFile(this->playback_input_handle, 
                            input,
                            sizeof(Platform_Input),
                            &bytes_read,
                            0);
    if(!success || bytes_read != sizeof(Platform_Input)) {
        return true;
    }
    return false;
}

#endif //TARGET_WIN32_RECORD_PLAYBACK_H
