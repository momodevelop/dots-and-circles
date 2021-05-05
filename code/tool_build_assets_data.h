/* date = April 21st 2021 6:01 pm */

#ifndef TOOL_BUILD_ASSETS_MSG_H
#define TOOL_BUILD_ASSETS_MSG_H


//~ NOTE(Momo): Atlas textures
struct tba_image_context {
    const char* Filename;
    image_id ImageId;
    texture_id TextureId;
};

static tba_image_context
Tba_ImageContexts[] = {
    {  "assets/ryoji.png",  Image_Ryoji, Texture_AtlasDefault },
    {  "assets/yuu.png",    Image_Yuu,    Texture_AtlasDefault },
    {  "assets/karu00.png", Image_Karu00, Texture_AtlasDefault },
    {  "assets/karu01.png", Image_Karu01, Texture_AtlasDefault },
    {  "assets/karu02.png", Image_Karu02, Texture_AtlasDefault },
    {  "assets/karu10.png", Image_Karu10, Texture_AtlasDefault },
    {  "assets/karu11.png", Image_Karu11, Texture_AtlasDefault },
    {  "assets/karu12.png", Image_Karu12, Texture_AtlasDefault },
    {  "assets/karu20.png", Image_Karu20, Texture_AtlasDefault },
    {  "assets/karu21.png", Image_Karu21, Texture_AtlasDefault },
    {  "assets/karu22.png", Image_Karu22, Texture_AtlasDefault },
    {  "assets/karu30.png", Image_Karu30, Texture_AtlasDefault },
    {  "assets/karu31.png", Image_Karu31, Texture_AtlasDefault },
    {  "assets/karu32.png", Image_Karu32, Texture_AtlasDefault },
    {  "assets/player_white.png", Image_PlayerDot, Texture_AtlasDefault },
    {  "assets/player_black.png", Image_PlayerCircle, Texture_AtlasDefault },
    {  "assets/bullet_dot.png", Image_BulletDot, Texture_AtlasDefault },
    {  "assets/bullet_circle.png", Image_BulletCircle, Texture_AtlasDefault },
    {  "assets/enemy.png", Image_Enemy, Texture_AtlasDefault },
};

//~ NOTE(Momo): Msg
struct tba_msg_context {
    msg_id Id;
    const char* Str;
};

static  tba_msg_context
Tba_MsgContexts[] = {
    Msg_ConsoleJumpToMain, "Jumping to Main",
    Msg_ConsoleJumpToMenu, "Jumping to Menu",
    Msg_ConsoleJumpToSandbox, "Jumping to Sandbox",
};

//~ NOTE(Momo): Sound
struct tba_sound_context {
    sound_id Id;
    const char* Filename;
};

static tba_sound_context
Tba_SoundContexts[] {
    Sound_Test, "assets/test_sound.wav",
    Sound_MainBgm, "assets/main_bgm.wav",
};

//~ NOTE(Momo): Animation
struct tba_anime_context {
    anime_id Id;
    image_id* Frames;
    u32 FrameCount;
};



// Karu_front
image_id KaruFrontFrames[] = {
    Image_Karu30,
    Image_Karu31,
    Image_Karu32,
};

static tba_anime_context
Tba_AnimeContexts[] = {
    Anime_KaruFront, KaruFrontFrames, ArrayCount(KaruFrontFrames),
};



#endif //TOOL_BUILD_ASSETS_MSG_H
