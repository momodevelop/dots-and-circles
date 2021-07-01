/* date = April 21st 2021 6:01 pm */

#ifndef TOOL_BUILD_ASSETS_MSG_H
#define TOOL_BUILD_ASSETS_MSG_H


//~ NOTE(Momo): Atlas textures
struct tba_image_context {
    const char* Filename;
    Image_ID ImageId;
    Texture_ID TextureId;
};

static tba_image_context
Tba_ImageContexts[] = {
    {  "assets/ryoji.png",  IMAGE_RYOJI, TEXTURE_ATLAS_DEFAULT },
    {  "assets/yuu.png",    IMAGE_YUU,    TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu00.png", IMAGE_KARU_00, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu01.png", Image_Karu01, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu02.png", Image_Karu02, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu10.png", Image_Karu10, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu11.png", Image_Karu11, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu12.png", Image_Karu12, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu20.png", Image_Karu20, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu21.png", Image_Karu21, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu22.png", Image_Karu22, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu30.png", Image_Karu30, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu31.png", Image_Karu31, TEXTURE_ATLAS_DEFAULT },
    {  "assets/karu32.png", Image_Karu32, TEXTURE_ATLAS_DEFAULT },
    {  "assets/player_white.png", Image_PlayerDot, TEXTURE_ATLAS_DEFAULT },
    {  "assets/player_black.png", Image_PlayerCircle, TEXTURE_ATLAS_DEFAULT },
    {  "assets/bullet_dot.png", Image_BulletDot, TEXTURE_ATLAS_DEFAULT },
    {  "assets/bullet_circle.png", Image_BulletCircle, TEXTURE_ATLAS_DEFAULT },
    {  "assets/enemy.png", Image_Enemy, TEXTURE_ATLAS_DEFAULT },
    {  "assets/particle.png", Image_Particle, TEXTURE_ATLAS_DEFAULT },
};

//~ NOTE(Momo): Msg
struct tba_msg_context {
    Msg_ID Id;
    const char* Str;
};

static  tba_msg_context
Tba_MsgContexts[] = {
    MSG_CONSOLE_JUMP_TO_MAIN, "Jumping to Main",
    MSG_CONSOLE_JUMP_TO_MENU, "Jumping to Menu",
    MSG_CONSOLE_JUMP_TO_SANDBOX, "Jumping to Sandbox",
};

//~ NOTE(Momo): Sound
struct tba_sound_context {
    Sound_ID Id;
    const char* Filename;
};

static tba_sound_context
Tba_SoundContexts[] {
    SOUND_TEST, "assets/test_sound.wav",
    SOUND_MAIN_BGM, "assets/main_bgm.wav",
};

//~ NOTE(Momo): Animation
struct tba_anime_context {
    Anime_ID Id;
    Image_ID* Frames;
    u32 FrameCount;
};



// Karu_front
Image_ID KaruFrontFrames[] = {
    Image_Karu30,
    Image_Karu31,
    Image_Karu32,
};

static tba_anime_context
Tba_AnimeContexts[] = {
    ANIME_KARU_FRONT, KaruFrontFrames, ARRAY_COUNT(KaruFrontFrames),
};



#endif //TOOL_BUILD_ASSETS_MSG_H
