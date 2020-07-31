#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "ryoji_render_commands.h"
#include "ryoji_maths.h"

#include "game_assets.h"


struct render_command_data_clear {
    static constexpr u32 TypeId = 0;
    c4f Colors;
};



static inline void
PushCommandClear(render_commands* Commands, c4f Colors) {
    using data_t = render_command_data_clear;
    auto* Data = PushCommand<data_t>(Commands);
    Data->Colors = Colors;
}

struct render_command_data_textured_quad {
    static constexpr u32 TypeId = 1;
    game_texture Texture;
    //u32 TextureHandle;
    c4f Colors;
    m44f Transform;
};
// TODO(Momo): Change TextureHandle and TextureData to use game_texture
static inline void
PushCommandTexturedQuad(render_commands* Commands, 
                        c4f Colors, 
                        m44f Transform, 
                        game_texture Texture) 
{
    using data_t = render_command_data_textured_quad;
    auto* Data = PushCommand<data_t>(Commands);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->Texture = Texture;
    
}

// TODO(Momo): A bit dangerous since I'm just transfering bitmap data by pointer.
// The pointer might be invalid during the transfer.
struct render_command_data_link_texture {
    static constexpr u32 TypeId = 2;
    game_texture Texture;
    //bitmap Bitmap;
    //u32 GameTextureHandle;
};

static inline void 
PushCommandLinkTexture(render_commands* Commands, game_texture Texture) {
    using data_t = render_command_data_link_texture;
    auto* Data = PushCommand<data_t>(Commands);
    Data->Texture = Texture;
}



#endif //GAME_RENDERER_H
