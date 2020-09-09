
static inline asset_builder_entry*
AddAssetEntry(asset_builder* Assets, asset_id Id, asset_type Type) {
    Assert(Assets->EntryCount != MAX_ENTRIES);
    asset_builder_entry* Ret = &Assets->Entries[Assets->EntryCount++];
    Ret->Id = Id;
    Ret->Type = Type;
    return Ret;
}


static inline void
AddImage(asset_builder* Assets, const char* Filename, asset_id Id) {
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Image);
    Entry->Image.Filename = Filename;
}

static inline void
AddFont(asset_builder* Assets, const char* Filename, asset_id Id, f32 Size, const char* Characters) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Font);
    Entry->Font.Filename = Filename;
    Entry->Font.Characters = Characters;
    Entry->Font.Size = Size;
}


static inline void 
AddSpritesheet(asset_builder* Assets, 
               const char* Filename, 
               asset_id Id,  
               u32 Rows, 
               u32 Columns) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Spritesheet);
    Entry->Spritesheet.Filename = Filename;
    Entry->Spritesheet.Rows = Rows;
    Entry->Spritesheet.Cols = Columns;
}



static inline b32 
WriteToFile(asset_builder* Assets, const char* Filename) {
    // TODO(Momo): Maybe write a writer struct for all these states
    FILE* OutFile = fopen(Filename, "wb");
    if (OutFile == nullptr) {
        printf("Cannot open %s\n", Filename);
        return false;
    }
    Defer { fclose(OutFile); };
    printf("%s opened!\n", Filename);
    
    usize HeaderAt = (usize)ftell(OutFile);
    
    
    // NOTE(Momo): Write signature
    fwrite(AssetSignature, sizeof(u8), sizeof(AssetSignature), OutFile);
    HeaderAt += sizeof(AssetSignature);
    
    // NOTE(Momo): Write the amount of items
    fwrite(&Assets->EntryCount, sizeof(Assets->EntryCount), 1, OutFile);
    HeaderAt += sizeof(Assets->EntryCount);
    
    usize DataAt = HeaderAt + (sizeof(asset_file_entry) * Assets->EntryCount);
    
    
    // NOTE(Momo): Write the data
    for (u32 i = 0; i < Assets->EntryCount; ++i)
    {
        asset_builder_entry* Entry = &Assets->Entries[i];
        
        switch(Entry->Type) {
            case AssetType_Image: {
                const char* ImageFilename = Entry->Image.Filename;
                i32 ImageWidth, ImageHeight, ImageComp;
                u8* LoadedImage = stbi_load(ImageFilename, &ImageWidth, &ImageHeight, &ImageComp, 0);
                if (LoadedImage == nullptr) {
                    printf("Error loading image: %s\n", ImageFilename);
                    return false;
                }
                Defer { stbi_image_free(LoadedImage); };
                
                // NOTE(Momo): Write Header
                fseek(OutFile, (i32)HeaderAt, SEEK_SET);
                {
                    asset_file_entry Header = {};
                    Header.Type = Entry->Type;
                    Header.OffsetToData = (u32)DataAt;
                    Header.Id = Entry->Id;
                    
                    fwrite(&Header, sizeof(asset_file_entry), 1, OutFile); 
                    HeaderAt += sizeof(asset_file_entry);
                }
                
                // NOTE(Momo): Write Data
                fseek(OutFile, (i32)DataAt, SEEK_SET);
                {
                    asset_file_data_image Image = {};
                    Image.Width = ImageWidth;
                    Image.Height = ImageHeight;
                    Image.Channels = ImageComp;
                    
                    fwrite(&Image, sizeof(asset_file_data_image), 1, OutFile); 
                    DataAt += sizeof(asset_file_data_image);
                    
                    u32 ImageSize = (ImageWidth * ImageHeight * ImageComp);
                    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) 
                    {
                        fwrite(Itr, 1, 1, OutFile); 
                        ++DataAt;
                    }
                }
                
                printf("Loaded Image '%s': Width = %d, Height = %d, Comp = %d\n", ImageFilename, ImageWidth, ImageHeight, ImageComp);
                
            } break;
            case AssetType_Spritesheet: {
                const char* ImageFilename = Entry->Spritesheet.Filename;
                i32 ImageWidth, ImageHeight, ImageComp;
                u8* LoadedImage = stbi_load(ImageFilename, &ImageWidth, &ImageHeight, &ImageComp, 0);
                if (LoadedImage == nullptr) {
                    printf("Error loading spritesheet %s\n", ImageFilename);
                    return false;
                }
                Defer { stbi_image_free(LoadedImage); };
                
                // NOTE(Momo): Write Header
                fseek(OutFile, (i32)HeaderAt, SEEK_SET);
                {
                    asset_file_entry Header = {};
                    Header.Type = AssetType_Spritesheet;
                    Header.OffsetToData = (u32)DataAt;
                    Header.Id = Entry->Id;
                    
                    fwrite(&Header, sizeof(asset_file_entry), 1, OutFile); 
                    HeaderAt += sizeof(asset_file_entry);
                }
                
                
                // NOTE(Momo): Write Data
                fseek(OutFile, (i32)DataAt, SEEK_SET);
                {
                    asset_file_data_spritesheet Spritesheet = {};
                    Spritesheet.Width = (u32)ImageWidth;
                    Spritesheet.Height = (u32)ImageHeight;
                    Spritesheet.Channels = (u32)ImageComp;
                    Spritesheet.Rows = Entry->Spritesheet.Rows;
                    Spritesheet.Cols = Entry->Spritesheet.Cols;
                    
                    fwrite(&Spritesheet, sizeof(asset_file_data_spritesheet), 1, OutFile); 
                    DataAt += sizeof(asset_file_data_spritesheet);
                    
                    u32 ImageSize = (ImageWidth * ImageHeight * ImageComp);
                    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) 
                    {
                        fwrite(Itr, 1, 1, OutFile); 
                        ++DataAt;
                    }
                }
                
                printf("Loaded Spritesheet '%s': Width = %d, Height = %d, Comp = %d, Rows = %d, Cols = %d\n", ImageFilename, ImageWidth, ImageHeight, ImageComp, Entry->Spritesheet.Rows, Entry->Spritesheet.Cols);
            } break;
            
            case AssetType_Font: {
                
                const char* FontFilename = Entry->Font.Filename;
                u8* FontBuffer = nullptr; 
                {
                    FILE* pFile = fopen(FontFilename, "rb");
                    if (pFile == nullptr) {
                        printf("Cannot open %s\n", FontFilename);
                        return false;
                    }
                    Defer { fclose(pFile); };
                    
                    fseek(pFile, 0, SEEK_END);
                    u32 size = ftell(pFile); 
                    fseek(pFile, 0, SEEK_SET); 
                    
                    FontBuffer = (u8*)malloc(size);
                    fread(FontBuffer, size, 1, pFile);
                }
                Defer { free(FontBuffer); };
                
                stbtt_fontinfo FontInfo;
                if (!stbtt_InitFont(&FontInfo, FontBuffer, 0)) {
                    return false;
                }
                
                f32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, Entry->Font.Size);
                u32 CharacterCount = StringLen(Entry->Font.Characters);
                
                auto* Stamps = (asset_file_data_font_character*)malloc(CharacterCount* sizeof(asset_file_data_font_character));
                Defer { free(Stamps); };
                
                i32 BitmapWidth = 0;
                i32 BitmapHeight = 0;
                const i32 Channels = 4;
                {
                    // NOTE(Momo): Bitmap is one row of characters, so Y is always zero. However, this will give me a good insight on how to upgrade this to a texture atlas system, so I'll leave it here for now. If we decide to make this POT, then Y will be of some use...
                    for (u32 j = 0; j < CharacterCount; ++j) {
                        i32 x1, y1, x2, y2;
                        stbtt_GetCodepointBitmapBox(&FontInfo, Entry->Font.Characters[j], Scale, Scale, &x1, &y1, &x2, &y2);
                        
                        Stamps[j].X = BitmapWidth;
                        Stamps[j].Y = 0;
                        Stamps[j].W = (x2 - x1);
                        Stamps[j].H = (y2 - y1);
                        Stamps[j].Codepoint = Entry->Font.Characters[j];
                        
                        BitmapWidth += (x2 - x1);
                        BitmapHeight = Maximum(y2 - y1, BitmapHeight);
                    }
                }
                
                // NOTE(Momo): Write Header
                fseek(OutFile, (i32)HeaderAt, SEEK_SET);
                {
                    asset_file_entry Header = {};
                    Header.Type = AssetType_Font;
                    Header.OffsetToData = (u32)DataAt;
                    Header.Id = Entry->Id;
                    
                    fwrite(&Header, sizeof(asset_file_entry), 1, OutFile); 
                    HeaderAt += sizeof(asset_file_entry);
                }
                
                
                // NOTE(Momo): Write Data
                fseek(OutFile, (i32)DataAt, SEEK_SET);
                {
                    
                    
                    asset_file_data_font Font = {};
                    Font.Width = (u32)BitmapWidth;
                    Font.Height = (u32)BitmapHeight;
                    Font.Channels = (u32)Channels;
                    Font.CharacterCount = (u32)CharacterCount;
                    
                    fwrite(&Font, sizeof(asset_file_data_font), 1, OutFile); 
                    DataAt += sizeof(asset_file_data_font);
                    
                    // NOTE(Momo): Data will be structured after header with:
                    // - Array of stamps
                    // - Bitmap Data
                    // Also, the bitmap is just going to be one row of characters
                    
                    
                    // NOTE(Momo): Write array of rects for each code point
                    for (u32 j = 0; j < CharacterCount; ++j) {
                        fwrite(&Stamps[j], sizeof(asset_file_data_font_character), 1, OutFile); 
                        DataAt += sizeof(asset_file_data_font_character);
                    }
                    
                    // NOTE(Momo): Write the bitmap data
                    {
                        u32 BitmapSize = BitmapWidth * BitmapHeight *  Channels;
                        u8* Bitmap = (u8*)calloc(BitmapSize, sizeof(u8));
                        Defer { free(Bitmap); };
                        for (u32 j = 0; j < CharacterCount; ++j) {
                            auto* Stamp = &Stamps[j];
                            u32 StampBitmapSize = Stamp->W * Stamp->H;
                            u8* StampBitmap = (u8*)malloc(StampBitmapSize);
                            Defer { free(StampBitmap); };
                            
                            stbtt_MakeCodepointBitmap(&FontInfo, StampBitmap, Stamp->W, Stamp->H, Stamp->W, Scale, Scale, Stamp->Codepoint);
                            i32 k = 0;
                            for (u32 y = Stamp->Y; y < Stamp->Y + Stamp->H; ++y) {
                                for (u32 x =  Stamp->X; x <  Stamp->X +  Stamp->W; ++x) 
                                {
                                    u32 Index = TwoToOne(y, x, BitmapWidth) * Channels;
                                    for (u32 c = 0; c < Channels; ++c) {
                                        Bitmap[Index + c] = StampBitmap[k]; 
                                    }
                                    ++k;
                                }
                                
                            }
                            
                        }
                        
                        for (u8* Itr = Bitmap; Itr < Bitmap + BitmapSize; ++Itr) 
                        {
                            fwrite(Itr, 1, 1, OutFile); 
                            ++DataAt;
                        }
#if 0
                        stbi_write_png("test.png", BitmapWidth, BitmapHeight, Channels, Bitmap, BitmapWidth  * Channels);
#endif
                    }
                    
                }
                
                printf("Loaded Font '%s': Width = %d, Height = %d\n", FontFilename, BitmapWidth, BitmapHeight);
                
                
                
            } break; 
        }
        
    }
    
    return true;
}