#ifndef __BMP5_32BPP_H__
#define __BMP5_32BPP_H__

#include <stdlib.h>
#include <stdio.h>
/***************************************
== File Header
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;

== Info Header
  DWORD        bV5Size;
  LONG         bV5Width;
  LONG         bV5Height;
  WORD         bV5Planes;
  WORD         bV5BitCount;
  DWORD        bV5Compression;
  DWORD        bV5SizeImage;
  LONG         bV5XPelsPerMeter;
  LONG         bV5YPelsPerMeter;
  DWORD        bV5ClrUsed;
  DWORD        bV5ClrImportant;
  DWORD        bV5RedMask;
  DWORD        bV5GreenMask;
  DWORD        bV5BlueMask;
  DWORD        bV5AlphaMask;
  DWORD        bV5CSType;
  CIEXYZTRIPLE bV5Endpoints;
  DWORD        bV5GammaRed;
  DWORD        bV5GammaGreen;
  DWORD        bV5GammaBlue;
  DWORD        bV5Intent;
  DWORD        bV5ProfileData;
  DWORD        bV5ProfileSize;
  DWORD        bV5Reserved;
******************************************/
#define MapTo16(buffer, index) (buffer[index] | (buffer[index+1] << 8))
#define MapTo32(buffer, index) (buffer[index] | (buffer[index+1] << 8) |  (buffer[index+2] << 16) | (buffer[index+3] << 24)) 

constexpr u8 kBmpFileHeaderSize = 14;
constexpr u8 kBmpInfoHeaderSize = 124;
constexpr u8 kBmpCompression = 3;
constexpr u8 kBmpBitsPerPixel = 32;
constexpr u16 kBmpSignature = 0x4D42;

struct BmpFileHeader {
    u16 Type;
    u32 Size;
    u16 Reserved1;
    u16 Reserved2;
    u32 OffsetBits;
}; // 14 bytes


struct BmpInfoHeader {
    u32 Size;
    i32 Width;
    i32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 ImageSize;
    i32 XPelsPerMeter;
    i32 YPelsPerMeter;
    u32 ColorUsed;
    u32 ColorImportant;
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
    u32 AlphaMask;
    u32 CSType;
    i32 RedX;         
    i32 RedY;         
    i32 RedZ;         
    i32 GreenX;       
    i32 GreenY;       
    i32 GreenZ;       
    i32 BlueX;        
    i32 BlueY;        
    i32 BlueZ;        
    u32 GammaRed;
    u32 GammaGreen;
    u32 GammaBlue;
    u32 Intent;
    u32 ProfileData;
    u32 ProfileSize;
    u32 Reserved;
}; // 124 bytes


// NOTE(Momo): 32-bit!
struct BmpPixel {
    u8  Red, Green, Blue, Alpha;
};


struct Bmp {
    BmpFileHeader FileHeader;
    BmpInfoHeader InfoHeader;
	BmpPixel *Pixels;
};

enum BmpError {
    BMP_ERR_NONE,
    BMP_ERR_FILE_CANNOT_OPEN,
    BMP_ERR_BAD_FILE_HEADER,
    BMP_ERR_BAD_INFO_HEADER,
    BMP_ERR_BAD_SIGNATURE,
    BMP_ERR_UNSUPPORTED_BPP,
    BMP_ERR_UNSUPPORTED_COMPRESSION,
    BMP_ERR_INVALID_PIXEL_DATA,
    BMP_ERR_OUT_OF_MEMORY,
    BMP_ERR_OFFSET_BITS_TOO_SMOL
};

const char* BmpErrorStr(BmpError err) {
    switch (err) {
        case BMP_ERR_NONE: return "BMP_ERR_NONE";
        case BMP_ERR_FILE_CANNOT_OPEN: return "BMP_ERR_FILE_CANNOT_OPEN";
        case BMP_ERR_BAD_FILE_HEADER: return "BMP_ERR_BAD_FILE_HEADER";
        case BMP_ERR_BAD_INFO_HEADER: return "BMP_ERR_BAD_INFO_HEADER";
        case BMP_ERR_BAD_SIGNATURE: return "BMP_ERR_BAD_SIGNATURE"; 
        case BMP_ERR_UNSUPPORTED_BPP: return "BMP_ERR_UNSUPPORTED_BPP";
        case BMP_ERR_UNSUPPORTED_COMPRESSION: return "BMP_ERR_UNSUPPORTED_COMPRESSION";
        case BMP_ERR_INVALID_PIXEL_DATA: return "BMP_ERR_INVALID_PIXEL_DATA";
        case BMP_ERR_OUT_OF_MEMORY: return "BMP_ERR_OUT_OF_MEMORY";
        case BMP_ERR_OFFSET_BITS_TOO_SMOL: return "BMP_ERR_OFFSET_BITS_TOO_SMOL";
    }
    return nullptr;
}

// NOTE(Momo): Initializes standard stuff to get things started
void Init(Bmp* bmp, i32 width, i32 height) {
    // NOTE(Momo): File Header
    bmp->FileHeader.Type = kBmpSignature;
    bmp->FileHeader.Size = kBmpFileHeaderSize + kBmpInfoHeaderSize + (width * height * kBmpBitsPerPixel/8);
    bmp->FileHeader.Reserved1 = bmp->FileHeader.Reserved2 = 0;
    bmp->FileHeader.OffsetBits = kBmpFileHeaderSize + kBmpInfoHeaderSize;
    
    // NOTE(Momo): Info Header
    bmp->InfoHeader.Size = kBmpInfoHeaderSize;
    bmp->InfoHeader.Width = width;
    bmp->InfoHeader.Height = height;
    bmp->InfoHeader.Planes = 1;
    bmp->InfoHeader.BitsPerPixel = kBmpBitsPerPixel;
    bmp->InfoHeader.Compression = kBmpCompression;
    bmp->InfoHeader.ImageSize = 0;
    bmp->InfoHeader.XPelsPerMeter = 0;
    bmp->InfoHeader.YPelsPerMeter = 0;
    bmp->InfoHeader.ColorUsed = 0;
    bmp->InfoHeader.ColorImportant = 0;
    bmp->InfoHeader.RedMask = 0x00FF0000;
    bmp->InfoHeader.GreenMask = 0x0000FF00;
    bmp->InfoHeader.BlueMask = 0x000000FF;
    bmp->InfoHeader.AlphaMask = 0xFF000000;
    bmp->InfoHeader.CSType = 0; 
    bmp->InfoHeader.RedX = 0;
    bmp->InfoHeader.RedY = 0;
    bmp->InfoHeader.RedZ = 0;
    bmp->InfoHeader.GreenX = 0;
    bmp->InfoHeader.GreenY = 0;
    bmp->InfoHeader.GreenZ = 0;
    bmp->InfoHeader.BlueX = 0;
    bmp->InfoHeader.BlueY = 0;
    bmp->InfoHeader.BlueZ = 0;
    bmp->InfoHeader.GammaRed = 0;
    bmp->InfoHeader.GammaGreen = 0;
    bmp->InfoHeader.GammaBlue = 0;
    bmp->InfoHeader.Intent = 0;
    bmp->InfoHeader.ProfileData = 0;
    bmp->InfoHeader.ProfileSize = 0;
    bmp->InfoHeader.Reserved = 0;
    
    bmp->Pixels = nullptr;
}

#if 1
#define Print(str, ...) printf(str, __VA_ARGS__)
#else
#define Print(str, ...)
#endif
// NOTE(Momo): Initialize from file
BmpError Load(Bmp* bmp, const char* path)  {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        return BMP_ERR_FILE_CANNOT_OPEN;
    }
    
    u8 bmpFileHeader[kBmpFileHeaderSize] = {};
    u8 bmpInfoHeader[kBmpInfoHeaderSize] = {};
    if (fread(bmpFileHeader, sizeof(bmpFileHeader), 1, file) == 0){
		fclose(file);
        return BMP_ERR_BAD_FILE_HEADER ;
    }
    
    if(fread(bmpInfoHeader, sizeof(bmpInfoHeader), 1, file) == 0) {
		fclose(file);
        return BMP_ERR_BAD_INFO_HEADER;
    }
    
    // NOTE(Momo): File Header
    bmp->FileHeader.Type = MapTo16(bmpFileHeader, 0); // Check signature
    if(bmp->FileHeader.Type != 0x4D42) { 
        fclose(file);
        return BMP_ERR_BAD_SIGNATURE;
    }
    
    bmp->FileHeader.Size = MapTo32(bmpFileHeader, 2);
    bmp->FileHeader.Reserved1 = bmp->FileHeader.Reserved2 = 0;
    bmp->FileHeader.OffsetBits = MapTo32(bmpFileHeader, 10);
    
    
    // NOTE(Momo): Info Header
    bmp->InfoHeader.Size = MapTo32(bmpInfoHeader,0);
    bmp->InfoHeader.Width = MapTo32(bmpInfoHeader, 4);
    bmp->InfoHeader.Height = MapTo32(bmpInfoHeader, 8);
    bmp->InfoHeader.Planes = MapTo16(bmpInfoHeader, 12);
    bmp->InfoHeader.BitsPerPixel = MapTo16(bmpInfoHeader, 14);
    if (bmp->InfoHeader.BitsPerPixel != kBmpBitsPerPixel) {
        fclose(file);
        return BMP_ERR_UNSUPPORTED_BPP;
    }
    
    bmp->InfoHeader.Compression = MapTo32(bmpInfoHeader, 16);
    if (bmp->InfoHeader.Compression != kBmpCompression) {
        fclose(file);
        return BMP_ERR_UNSUPPORTED_COMPRESSION;
    }
    
    bmp->InfoHeader.ImageSize = MapTo32(bmpInfoHeader, 20);
    bmp->InfoHeader.XPelsPerMeter = MapTo32(bmpInfoHeader, 24);
    bmp->InfoHeader.YPelsPerMeter = MapTo32(bmpInfoHeader, 28);
    bmp->InfoHeader.ColorUsed = MapTo32(bmpInfoHeader, 32);
    bmp->InfoHeader.ColorImportant = MapTo32(bmpInfoHeader, 36);
    bmp->InfoHeader.RedMask = MapTo32(bmpInfoHeader, 40);
    bmp->InfoHeader.GreenMask = MapTo32(bmpInfoHeader, 44);
    bmp->InfoHeader.BlueMask = MapTo32(bmpInfoHeader,48);
    bmp->InfoHeader.AlphaMask = MapTo32(bmpInfoHeader,52);
    bmp->InfoHeader.CSType = MapTo32(bmpInfoHeader, 56); // TODO(Momo): Check for other types?
    bmp->InfoHeader.RedX = MapTo32(bmpInfoHeader, 60);
    bmp->InfoHeader.RedY = MapTo32(bmpInfoHeader, 64);
    bmp->InfoHeader.RedZ = MapTo32(bmpInfoHeader, 68);
    bmp->InfoHeader.GreenX = MapTo32(bmpInfoHeader, 72);
    bmp->InfoHeader.GreenY = MapTo32(bmpInfoHeader, 76);
    bmp->InfoHeader.GreenZ = MapTo32(bmpInfoHeader, 80);
    bmp->InfoHeader.BlueX = MapTo32(bmpInfoHeader, 84);
    bmp->InfoHeader.BlueY = MapTo32(bmpInfoHeader, 88);
    bmp->InfoHeader.BlueZ = MapTo32(bmpInfoHeader, 92);
    bmp->InfoHeader.GammaRed = MapTo32(bmpInfoHeader, 96);
    bmp->InfoHeader.GammaGreen = MapTo32(bmpInfoHeader, 100);
    bmp->InfoHeader.GammaBlue = MapTo32(bmpInfoHeader, 104);
    bmp->InfoHeader.Intent = MapTo32(bmpInfoHeader, 108);
    bmp->InfoHeader.ProfileData = MapTo32(bmpInfoHeader, 112);
    bmp->InfoHeader.ProfileSize = MapTo32(bmpInfoHeader, 116);
    bmp->InfoHeader.Reserved = MapTo32(bmpInfoHeader, 120);
    
    
    // pad until we reach 
    // NOTE(Momo): pixel data
    u32 pixelAmt = bmp->InfoHeader.Width * bmp->InfoHeader.Height;
    bmp->Pixels = (BmpPixel*)malloc(pixelAmt * sizeof(BmpPixel));
    
    if (bmp->Pixels == nullptr) {
        fclose(file);
        return BMP_ERR_OUT_OF_MEMORY;
    }
    
    // Go to pixel data
    fseek(file, bmp->FileHeader.OffsetBits, SEEK_SET);
    
    BmpPixel* itr = bmp->Pixels;
    for (u32 i = 0; i < pixelAmt; ++i) {
        u32 pixelData, color, mask;
        if(fread(&pixelData, sizeof(u32), 1, file) == 0) {
            fclose(file);
            return BMP_ERR_INVALID_PIXEL_DATA;
        }
        
        mask = bmp->InfoHeader.RedMask;
        if (mask > 0) {
            color = pixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            itr->Red = (u8)color;
        }
        
        mask = bmp->InfoHeader.GreenMask;
        if (mask > 0) {
            color = pixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            itr->Green = (u8)color;
        }
        
        mask = bmp->InfoHeader.BlueMask;
        if (mask > 0) {
            color = pixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            itr->Blue = (u8)color;
        }
        
        mask = bmp->InfoHeader.AlphaMask;
        if(mask > 0) {
            color = pixelData & mask;
            while( color != 0 && (mask & 1) == 0 ) mask >>= 1, color >>= 1;
            itr->Alpha = (u8)color;
        }
        ++itr;
    }
    
    
    return BMP_ERR_NONE;
}


void Unload(Bmp * bmp) {
    free(bmp->Pixels);
}


// only for 32-bit
#define WriteVar(prop, file) fwrite(&prop, sizeof(prop), 1, file)
BmpError Write(const Bmp* bmp, const char* path) {
    if (bmp->FileHeader.Type != kBmpSignature) {
        return BMP_ERR_BAD_SIGNATURE;
    }
    
    if (bmp->InfoHeader.Compression != kBmpCompression) {
        return BMP_ERR_UNSUPPORTED_COMPRESSION;
    }
    
    if (bmp->InfoHeader.BitsPerPixel != kBmpBitsPerPixel) {
        return BMP_ERR_UNSUPPORTED_BPP;
    }
    
    if (bmp->FileHeader.OffsetBits < (kBmpInfoHeaderSize + kBmpFileHeaderSize)) {
        return BMP_ERR_OFFSET_BITS_TOO_SMOL;
    }
    
    
    FILE* file = fopen(path, "w+b");
    if (file == nullptr) {
        return BMP_ERR_FILE_CANNOT_OPEN;
    }
    
    WriteVar(bmp->FileHeader.Type, file); 
    WriteVar(bmp->FileHeader.Size, file);
    WriteVar(bmp->FileHeader.Reserved1, file);
    WriteVar(bmp->FileHeader.Reserved2, file);
    WriteVar(bmp->FileHeader.OffsetBits, file);
    
    WriteVar(bmp->InfoHeader.Size, file);
    WriteVar(bmp->InfoHeader.Width, file);
    WriteVar(bmp->InfoHeader.Height, file);
    WriteVar(bmp->InfoHeader.Planes, file);
    WriteVar(bmp->InfoHeader.BitsPerPixel, file);
    WriteVar(bmp->InfoHeader.Compression, file);
    WriteVar(bmp->InfoHeader.ImageSize, file);
    WriteVar(bmp->InfoHeader.XPelsPerMeter, file);
    WriteVar(bmp->InfoHeader.YPelsPerMeter, file);
    WriteVar(bmp->InfoHeader.ColorUsed, file);
    WriteVar(bmp->InfoHeader.ColorImportant, file);
    WriteVar(bmp->InfoHeader.RedMask, file);
    WriteVar(bmp->InfoHeader.GreenMask, file);
    WriteVar(bmp->InfoHeader.BlueMask, file);
    WriteVar(bmp->InfoHeader.AlphaMask, file);
    WriteVar(bmp->InfoHeader.CSType, file);
    WriteVar(bmp->InfoHeader.RedX, file);
    WriteVar(bmp->InfoHeader.RedY, file);
    WriteVar(bmp->InfoHeader.RedZ, file);
    WriteVar(bmp->InfoHeader.GreenX, file);
    WriteVar(bmp->InfoHeader.GreenY, file);
    WriteVar(bmp->InfoHeader.GreenZ, file);
    WriteVar(bmp->InfoHeader.BlueX, file);
    WriteVar(bmp->InfoHeader.BlueY, file);
    WriteVar(bmp->InfoHeader.BlueZ, file);
    WriteVar(bmp->InfoHeader.GammaRed, file);
    WriteVar(bmp->InfoHeader.GammaGreen, file);
    WriteVar(bmp->InfoHeader.GammaBlue, file);
    WriteVar(bmp->InfoHeader.Intent, file);
    WriteVar(bmp->InfoHeader.ProfileData, file);
    WriteVar(bmp->InfoHeader.ProfileSize, file);
    WriteVar(bmp->InfoHeader.Reserved, file);
    
    
    // pad until we reach the pixel data location 
    u32 diff = bmp->FileHeader.OffsetBits - kBmpInfoHeaderSize - kBmpFileHeaderSize;
    for (u32 i = 0; i < diff; ++i) {
        u8 tmp = 0;
        WriteVar(tmp, file);
    }
    
    
    
    u32 pixelAmt = bmp->InfoHeader.Width * bmp->InfoHeader.Height;
    for (u32 i = 0; i < pixelAmt; ++i) {
        u32 color = 0, mask = 0;
        
        mask = bmp->InfoHeader.RedMask;
        if (mask > 0) {
            u32 shiftAmt = 0;  
            while( (mask & 1) == 0) mask >>= 1, ++shiftAmt;
            color |= bmp->Pixels[i].Red << shiftAmt;
        }
        
        mask = bmp->InfoHeader.GreenMask;
        if (mask > 0) {
            u32 shiftAmt = 0;  
            while( (mask & 1) == 0) mask >>= 1, ++shiftAmt;
            color |= bmp->Pixels[i].Green << shiftAmt;
        }
        
        mask = bmp->InfoHeader.BlueMask;
        if (mask > 0) {
            u32 shiftAmt = 0;  
            while( (mask & 1) == 0) mask >>= 1, ++shiftAmt;
            color |= bmp->Pixels[i].Blue << shiftAmt;
        }
        
        mask = bmp->InfoHeader.AlphaMask;
        if (mask > 0) {
            u32 shiftAmt = 0;  
            while( (mask & 1) == 0) mask >>= 1, ++shiftAmt;
            color |= bmp->Pixels[i].Alpha << shiftAmt;
        }
        
        WriteVar(color, file);
        
        
    }
    
    fclose(file);
    return BMP_ERR_NONE;
}
#undef WriteVar
#undef MapTo16
#undef MapTo32

#endif // __BMP32_H__