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
AddAtlas(asset_builder* Assets, const char* Filename, asset_id Id) 
{
    asset_builder_entry* Entry = AddAssetEntry(Assets, Id, AssetType_Atlas);
    Entry->Atlas.Filename = Filename;
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

static inline u32 
WriteSpritesheetData (asset_builder* Assets, FILE* File, asset_builder_entry_spritesheet* Spritesheet) 
{
    u32 BytesWritten = 0;
    i32 Width, Height, Comp;
    u8* Loaded = stbi_load(Spritesheet->Filename, &Width, &Height, &Comp, 0);
    Assert(Loaded != nullptr);
    Defer { stbi_image_free(Loaded); };
    
    // NOTE(Momo): Write Data
    asset_file_data_spritesheet FileSpritesheet = {};
    FileSpritesheet.Width = (u32)Width;
    FileSpritesheet.Height = (u32)Height;
    FileSpritesheet.Channels = (u32)Comp;
    FileSpritesheet.Rows = Spritesheet->Rows;
    FileSpritesheet.Cols = Spritesheet->Cols;
    
    fwrite(&FileSpritesheet, sizeof(asset_file_data_spritesheet), 1, File); 
    BytesWritten+= sizeof(asset_file_data_spritesheet);
    
    u32 Size = (Width * Height * Comp);
    for (u8* Itr = Loaded; Itr < Loaded + Size; ++Itr) 
    {
        fwrite(Itr, 1, 1, File); 
        ++BytesWritten;
    }
    
    printf("Loaded Spritesheet '%s': Width = %d, Height = %d, Comp = %d, Rows = %d, Cols = %d\n", Spritesheet->Filename, Width, Height, Comp, Spritesheet->Rows, Spritesheet->Cols);
    
    return BytesWritten;
    
}

static inline u32 
WriteImageData(asset_builder* Assets, FILE* File, asset_builder_entry_image* Image) 
{
    u32 BytesWritten = 0;
    i32 Width, Height, Comp;
    u8* LoadedImage = stbi_load(Image->Filename, &Width, &Height, &Comp, 0);
    Assert(LoadedImage != nullptr);
    Defer { stbi_image_free(LoadedImage); };
    
    asset_file_data_image FileImage = {};
    FileImage.Width = Width;
    FileImage.Height = Height;
    FileImage.Channels = Comp;
    
    // NOTE(Momo): Write the data info
    fwrite(&FileImage, sizeof(asset_file_data_image), 1, File); 
    BytesWritten += sizeof(asset_file_data_image);
    
    // NOTE(Momo): Write raw data
    u32 ImageSize = Width * Height * Comp;
    for (u8* Itr = LoadedImage; Itr < LoadedImage + ImageSize; ++Itr) {
        fwrite(Itr, 1, 1, File); 
        ++BytesWritten;
    }
    
    printf("Loaded Image '%s': Width = %d, Height = %d, Comp = %d\n", Image->Filename, Width, Height, Comp);
    
    return BytesWritten;
}

static inline u32
WriteFontData(asset_builder* Assets, FILE* File, asset_builder_entry_font* Font) 
{
    u32 WrittenBytes = 0;
    u8* FontBuffer = nullptr; 
    {
        FILE* FontFile = fopen(Font->Filename, "rb");
        Assert(FontFile);
        Defer { fclose(FontFile); };
        
        fseek(FontFile, 0, SEEK_END);
        u32 size = ftell(FontFile); 
        fseek(FontFile, 0, SEEK_SET); 
        
        FontBuffer = (u8*)malloc(size);
        fread(FontBuffer, size, 1, FontFile);
    }
    Defer { free(FontBuffer); };
    
    stbtt_fontinfo FontInfo;
    Assert(stbtt_InitFont(&FontInfo, FontBuffer, 0));
    
    f32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, Font->Size);
    u32 CharacterCount = NtsLength(Font->Characters);
    
    auto* Stamps = (asset_file_data_font_character*)malloc(CharacterCount* sizeof(asset_file_data_font_character));
    Defer { free(Stamps); };
    
    i32 BitmapWidth = 0;
    i32 BitmapHeight = 0;
    const i32 Channels = 4;
    {
        // NOTE(Momo): Bitmap is one row of characters, so Y is always zero. However, this will give me a good insight on how to upgrade this to a texture atlas system, so I'll leave it here for now. If we decide to make this POT, then Y will be of some use...
        for (u32 j = 0; j < CharacterCount; ++j) {
            i32 x1, y1, x2, y2;
            stbtt_GetCodepointBitmapBox(&FontInfo, Font->Characters[j], Scale, Scale, &x1, &y1, &x2, &y2);
            
            Stamps[j].X = BitmapWidth;
            Stamps[j].Y = 0;
            Stamps[j].W = (x2 - x1);
            Stamps[j].H = (y2 - y1);
            Stamps[j].Codepoint = Font->Characters[j];
            
            BitmapWidth += (x2 - x1);
            BitmapHeight = Maximum(y2 - y1, BitmapHeight);
        }
    }
    
    // NOTE(Momo): Write Data
    {
        asset_file_data_font FileFont = {};
        FileFont.Width = (u32)BitmapWidth;
        FileFont.Height = (u32)BitmapHeight;
        FileFont.Channels = (u32)Channels;
        FileFont.CharacterCount = (u32)CharacterCount;
        
        fwrite(&Font, sizeof(asset_file_data_font), 1, File); 
        WrittenBytes += sizeof(asset_file_data_font);
        
        // NOTE(Momo): Data will be structured after header with:
        // - Array of stamps
        // - Bitmap Data
        // Also, the bitmap is just going to be one row of characters
        
        
        // NOTE(Momo): Write array of rects for each code point
        for (u32 j = 0; j < CharacterCount; ++j) {
            fwrite(&Stamps[j], sizeof(asset_file_data_font_character), 1, File); 
            WrittenBytes += sizeof(asset_file_data_font_character);
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
                fwrite(Itr, 1, 1, File); 
                ++WrittenBytes;
            }
#if 0
            stbi_write_png("test.png", BitmapWidth, BitmapHeight, Channels, Bitmap, BitmapWidth  * Channels);
#endif
        }
        
    }
    
    printf("Loaded Font '%s': Width = %d, Height = %d\n", Font->Filename, BitmapWidth, BitmapHeight);
    return WrittenBytes;
    
}


static inline u32 
WriteAtlasData(asset_builder* Assets, FILE* File, asset_builder_entry_atlas* Atlas) {
    // NOTE(Momo): Just copy raw data
    u32 BytesWritten = 0;
    
    FILE* AtlasFile = fopen(Atlas->Filename, "rb");
    Assert(AtlasFile != nullptr);
    Defer { fclose(AtlasFile); };
    
    fseek (AtlasFile, 0, SEEK_END);
    u32 AtlasFileSize = (u32)ftell(AtlasFile);
    fseek (AtlasFile, 0, SEEK_SET);
    
    for (u32 i = 0; i < AtlasFileSize; ++i) {
        u8 Byte;
        fread(&Byte, 1, 1, AtlasFile);
        fwrite(&Byte, 1, 1, File); 
        ++BytesWritten;
    }
    
    printf("Loaded Atlas '%s': Filesize = %d\n", Atlas->Filename, AtlasFileSize);
    
    return BytesWritten;
    
    
}

static inline b32 
Write(asset_builder* Assets, const char* Filename) {
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
        switch(Entry->Type) {
            case AssetType_Image: {
                DataAt += WriteImageData(Assets, OutFile, &Entry->Image);
            } break;
            case AssetType_Spritesheet: {
                DataAt += WriteSpritesheetData(Assets, OutFile, &Entry->Spritesheet);
            } break;
            case AssetType_Font: {
                DataAt += WriteFontData(Assets, OutFile, &Entry->Font);
            } break;
            case AssetType_Atlas: {
                DataAt += WriteAtlasData(Assets, OutFile, &Entry->Atlas);
            } break;
            default: {
                Assert(false);
            }
        }
        
    }
    
    return true;
}