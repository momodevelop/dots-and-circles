#ifndef GAME_ASSETS_TYPE_ID 
#define GAME_ASSETS_TYPE_ID

#include "ryoji_maths.h"

enum bitmap_id : u32 {
    Bitmap_Ryoji,
    Bitmap_Yuu,
    Bitmap_AtlasDefault,
    
    Bitmap_Count,
};


enum atlas_rect_id : u32 {
    AtlasRect_Ryoji,
    AtlasRect_Yuu,
    AtlasRect_Karu00,
    AtlasRect_Karu01,
    AtlasRect_Karu02,
    AtlasRect_Karu10,
    AtlasRect_Karu11,
    AtlasRect_Karu12,
    AtlasRect_Karu20,
    AtlasRect_Karu21,
    AtlasRect_Karu22,
    AtlasRect_Karu30,
    AtlasRect_Karu31,
    AtlasRect_Karu32,
    
    AtlasRect_a,
    AtlasRect_b,
    AtlasRect_c,
    AtlasRect_d,
    AtlasRect_e,
    AtlasRect_f,
    AtlasRect_g,
    AtlasRect_h,
    AtlasRect_i,
    AtlasRect_j,
    AtlasRect_k,
    AtlasRect_l,
    AtlasRect_m,
    AtlasRect_n,
    AtlasRect_o,
    AtlasRect_p,
    AtlasRect_q,
    AtlasRect_r,
    AtlasRect_s,
    AtlasRect_t,
    AtlasRect_u,
    AtlasRect_v,
    AtlasRect_w,
    AtlasRect_x,
    AtlasRect_y,
    AtlasRect_z,
    
    AtlasRect_A,
    AtlasRect_B,
    AtlasRect_C,
    AtlasRect_D,
    AtlasRect_E,
    AtlasRect_F,
    AtlasRect_G,
    AtlasRect_H,
    AtlasRect_I,
    AtlasRect_J,
    AtlasRect_K,
    AtlasRect_L,
    AtlasRect_M,
    AtlasRect_N,
    AtlasRect_O,
    AtlasRect_P,
    AtlasRect_Q,
    AtlasRect_R,
    AtlasRect_S,
    AtlasRect_T,
    AtlasRect_U,
    AtlasRect_V,
    AtlasRect_W,
    AtlasRect_X,
    AtlasRect_Y,
    AtlasRect_Z,
    
    AtlasRect_Count,
};

enum asset_type : u32 {
    AssetType_Bitmap,
    AssetType_AtlasRect,
};


// NOTE(Momo): asset pack file structures
#pragma pack(push, 1)
struct yuu_entry {
    asset_type Type;
};

struct yuu_bitmap {
    bitmap_id Id;
    u32 Width;
    u32 Height;
    u32 Channels;
    // NOTE(Momo): Data is:
    /* 
u8 Pixels[Width * Height * Channels]; 
*/
};


struct yuu_atlas_rect {
    atlas_rect_id Id;
    bitmap_id BitmapId;
    rect2u Rect;
    
};

#pragma pack(pop)

#endif 