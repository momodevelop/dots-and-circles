#ifndef __GAME_DRAW__
#define __GAME_DRAW__



static inline void
DrawText(mailbox* RenderCommands, 
         game_assets* Assets,
         font_id FontId,
         v3f Position,
         u8_cstr String,
         f32 Size, 
         c4f Color = Color_White) 
{
    v3f CurPosition = Position;
    game_asset_font* Font = GameAssets_GetFont(Assets, FontId);
    
    f32 ZLayerOffset = 0.f;
    for(u32 I = 0; I < String.Size; ++I) {
        game_asset_font_glyph* Glyph = GameAssets_GetFontGlyph(Font, String.Data[I]);
        aabb2f Box = Glyph->Box; 
        
        // NOTE(Momo): Set bottom left as origin
        m44f A = M44f_Translation(0.5f, 0.5f, 0.f); 
        
        f32 BoxWidth = Aabb2f_Width(Box);
        f32 BoxHeight = Aabb2f_Height(Box);
        
        m44f S = M44f_Scale(BoxWidth * Size, 
                            BoxHeight* Size, 
                            1.f);
        
        m44f T = M44f_Translation(CurPosition.X + Box.Min.X * Size, 
                                  CurPosition.Y + Box.Min.Y * Size,  
                                  CurPosition.Z + ZLayerOffset);
        
        m44f SA = M44f_Concat(S,A);
        m44f TSA = M44f_Concat(T, SA);
        
        PushDrawTexturedQuad(RenderCommands, 
                             Color, 
                             TSA,
                             GameAssets_GetTexture(Assets, Glyph->TextureId)->Handle,
                             GetAtlasUV(Assets, Glyph));
        
        CurPosition.X += Glyph->Advance * Size;
        if (I != String.Size - 1 ) {
            u32 Kerning = GameAssets_GetKerning(Font, 
                                                String.Data[I], 
                                                String.Data[I+1]);
            CurPosition.X += Kerning * Size;
            
        }
        ZLayerOffset += 0.001f;
        
    }
    
    
    
}

#endif

