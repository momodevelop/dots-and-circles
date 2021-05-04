#ifndef GAME_H
#define GAME_H

#include "momo.h"

#if INTERNAL
static struct platform_api* G_Platform = {};
#endif 

#include "game_renderer.h"
#include "game_platform.h"

#include "game_assets_file.h"
#include "game_assets.h"

#include "game_draw.h"
#include "game_camera.h"
#include "game_audio_mixer.h"
#include "game_debug_inspector.h"
#include "game_debug_console.h"


#include "game_mode.h"
#include "game_mode_main.h"
#include "game_mode_splash.h"
#include "game_mode_sandbox.h"
#include "game_mode_anime_test.h"

#endif //GAME_H
