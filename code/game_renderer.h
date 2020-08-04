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
    u32 TextureHandle;
    c4f Colors;
    m44f Transform;
    
    quad2f TextureCoords; 
};


static inline void
PushCommandTexturedQuad(render_commands* Commands, 
                        c4f Colors, 
                        m44f Transform, 
                        u32 TextureHandle,
                        quad2f TextureCoords) 
{
    using data_t = render_command_data_textured_quad;
    auto* Data = PushCommand<data_t>(Commands);
    
    Data->Colors = Colors;
    Data->Transform = Transform;
    Data->TextureHandle = TextureHandle;
    Data->TextureCoords = TextureCoords;
}

struct render_command_data_link_texture {
    static constexpr u32 TypeId = 2;
    bitmap TextureBitmap;
    u32 TextureHandle;
};


static inline void 
PushCommandLinkTexture(render_commands* Commands, 
                       bitmap TextureBitmap, 
                       u32 TextureHandle) {
    using data_t = render_command_data_link_texture;
    auto* Data = PushCommand<data_t>(Commands);
    Data->TextureBitmap = TextureBitmap;
    Data->TextureHandle = TextureHandle;
}

#endif //GAME_RENDERER_H
