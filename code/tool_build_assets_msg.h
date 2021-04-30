/* date = April 21st 2021 6:01 pm */

#ifndef TOOL_BUILD_ASSETS_MSG_H
#define TOOL_BUILD_ASSETS_MSG_H


//~ NOTE(Momo): Atlas textures
struct tba_atlas_aabb_context {
    const char* Filename;
    atlas_aabb_id AtlasAabbId;
    texture_id TextureId;
};

static tba_atlas_aabb_context
Tba_AtlasAabbContexts[] = {
    {  "assets/ryoji.png",  AtlasAabb_Ryoji, Texture_AtlasDefault },
    {  "assets/yuu.png",    AtlasAabb_Yuu,    Texture_AtlasDefault },
    {  "assets/karu00.png", AtlasAabb_Karu00, Texture_AtlasDefault },
    {  "assets/karu01.png", AtlasAabb_Karu01, Texture_AtlasDefault },
    {  "assets/karu02.png", AtlasAabb_Karu02, Texture_AtlasDefault },
    {  "assets/karu10.png", AtlasAabb_Karu10, Texture_AtlasDefault },
    {  "assets/karu11.png", AtlasAabb_Karu11, Texture_AtlasDefault },
    {  "assets/karu12.png", AtlasAabb_Karu12, Texture_AtlasDefault },
    {  "assets/karu20.png", AtlasAabb_Karu20, Texture_AtlasDefault },
    {  "assets/karu21.png", AtlasAabb_Karu21, Texture_AtlasDefault },
    {  "assets/karu22.png", AtlasAabb_Karu22, Texture_AtlasDefault },
    {  "assets/karu30.png", AtlasAabb_Karu30, Texture_AtlasDefault },
    {  "assets/karu31.png", AtlasAabb_Karu31, Texture_AtlasDefault },
    {  "assets/karu32.png", AtlasAabb_Karu32, Texture_AtlasDefault },
    {  "assets/player_white.png", AtlasAabb_PlayerDot, Texture_AtlasDefault },
    {  "assets/player_black.png", AtlasAabb_PlayerCircle, Texture_AtlasDefault },
    {  "assets/bullet_dot.png", AtlasAabb_BulletDot, Texture_AtlasDefault },
    {  "assets/bullet_circle.png", AtlasAabb_BulletCircle, Texture_AtlasDefault },
    {  "assets/enemy.png", AtlasAabb_Enemy, Texture_AtlasDefault },
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
    Sound_Test, "test_sound.wav",
};

//~ NOTE(Momo): Animation
struct tba_anime_context {
    anime_id Id;
    atlas_aabb_id* Frames;
    u32 FrameCount;
};



// Karu_front
atlas_aabb_id KaruFrontFrames[] = {
    AtlasAabb_Karu30,
    AtlasAabb_Karu31,
    AtlasAabb_Karu32,
};

static tba_anime_context
Tba_AnimeContexts[] = {
    Anime_KaruFront, KaruFrontFrames, ArrayCount(KaruFrontFrames),
};



#endif //TOOL_BUILD_ASSETS_MSG_H
