/* date = April 21st 2021 6:01 pm */

#ifndef TOOL_BUILD_ASSETS_MSG_H
#define TOOL_BUILD_ASSETS_MSG_H


//~ NOTE(Momo): Atlas textures
struct Asset_Builder_Image_Context {
    const char* filename;
    Image_ID image_id;
    Texture_ID texture_id;
};

static Asset_Builder_Image_Context 
g_image_contexts[] = {
    {  "assets/ryoji.png",  IMAGE_RYOJI, TEXTURE_ATLAS_DEFAULT },
    {  "assets/yuu.png",    IMAGE_YUU,    TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu00.png", IMAGE_KARU_00, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu01.png", IMAGE_KARU_01, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu02.png", IMAGE_KARU_02, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu10.png", IMAGE_KARU_10, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu11.png", IMAGE_KARU_11, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu12.png", IMAGE_KARU_12, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu20.png", IMAGE_KARU_20, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu21.png", IMAGE_KARU_21, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu22.png", IMAGE_KARU_22, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu30.png", IMAGE_KARU_30, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu31.png", IMAGE_KARU_31, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu32.png", IMAGE_KARU_32, TEXTURE_ATLAS_DEFAULT },
    {  "assets/player_white.png", IMAGE_PLAYER_DOT, TEXTURE_ATLAS_DEFAULT },
    {  "assets/player_black.png", IMAGE_PLAYER_CIRCLE, TEXTURE_ATLAS_DEFAULT },
    {  "assets/bullet_dot.png", IMAGE_BULLET_DOT, TEXTURE_ATLAS_DEFAULT },
    {  "assets/bullet_circle.png", IMAGE_BULLET_CIRCLE, TEXTURE_ATLAS_DEFAULT },
    {  "assets/enemy.png", IMAGE_ENEMY, TEXTURE_ATLAS_DEFAULT },
    {  "assets/particle.png", IMAGE_PARTICLE, TEXTURE_ATLAS_DEFAULT },
};

//~ NOTE(Momo): Msg
struct Asset_Builder_Msg_Context {
    Msg_ID id;
    const char* str;
};

static  Asset_Builder_Msg_Context
g_msg_contexts[] = {
    MSG_CONSOLE_JUMP_TO_MAIN, "Jumping to Main",
    MSG_CONSOLE_JUMP_TO_MENU, "Jumping to Menu",
    MSG_CONSOLE_JUMP_TO_SANDBOX, "Jumping to Sandbox",
};

//~ NOTE(Momo): Sound
struct Asset_Builder_Sound_Context {
    Sound_ID id;
    const char* filename;
};

static Asset_Builder_Sound_Context
g_sound_contexts[] {
    SOUND_TEST, "assets/test_sound.wav",
    SOUND_MAIN_BGM, "assets/main_bgm.wav",
};

//~ NOTE(Momo): Animation
struct Asset_Builder_Anime_Context {
    Anime_ID id;
    Image_ID* frames;
    u32 frame_count;
};



// Karu_front
Image_ID karu_front_frames[] = {
    IMAGE_KARU_30,
    IMAGE_KARU_31,
    IMAGE_KARU_32,
};

static Asset_Builder_Anime_Context
g_anime_contexts[] = {
    ANIME_KARU_FRONT, karu_front_frames, ARRAY_COUNT(karu_front_frames),
};



#endif //TOOL_BUILD_ASSETS_MSG_H
