/* date = June 11th 2021 0:30 pm */

#ifndef GAME_GLOBALS_EXTERNAL_H
#define GAME_GLOBALS_EXTERNAL_H


// NOTE(Momo): Globals related to things from platform layer
platform_api* G_Platform;
platform_log* G_Log;
mailbox* G_Renderer;
platform_input* G_Input;
Arena* G_Scratch;


// NOTE(Momo): Game related globals
// Need to forward declare these
struct assets;
assets* G_Assets;

#endif //GAME_GLOBALS_H
