#ifndef __RYOJI_BITFIELDS__
#define __RYOJI_BITFIELDS__

// NOTE(Momo): Bitfields
pure u8 MaskBits(u8 flag, u8 mask) { 
    return flag | mask; 
}

pure u16 MaskBits(u16 flag, u16 mask) { 
    return flag | mask;
}

pure u32 MaskBits(u32 flag, u32 mask) { 
    return flag | mask; 
}

pure u64 MaskBits(u64 flag, u64 mask) {
    return flag | mask; 
}

pure u8 UnmaskBits(u8 flag, u8 mask) 	{ 
    return flag & ~mask; 
}

pure u16 UnmaskBits(u16 flag, u16 mask)  { 
    return flag & ~mask; 
}

pure u32 UnmaskBits(u32 flag, u32 mask)  {
    return flag & ~mask; 
}

pure u64 UnmaskBits(u64 flag, u64 mask)  {
    return flag & ~mask; 
}

pure u8 SetBits(u8 flag, usize index, bool val)  {
    Assert((sizeof(u8) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u16 SetBits(u16 flag, usize index, bool val)  {
    Assert((sizeof(u16) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u32 SetBits(u32 flag, usize index, bool val)  {
    Assert((sizeof(u32) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u64 SetBits(u64 flag, usize index, bool val)  {
    Assert((sizeof(u64) * 8) > (index - 1));
    return (flag & ~(1ULL << index)) | (val << index);
}

pure u8 GetBits(u8 flag, usize index, bool val)  {
    Assert((sizeof(u8) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u16 GetBits(u16 flag, usize index, bool val)  {
    Assert((sizeof(u16) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u32 GetBits(u32 flag, usize index, bool val)  {
    Assert((sizeof(u32) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u64 GetBits(u64 flag, usize index, bool val)  {
    Assert((sizeof(u64) * 8) > (index - 1));
    return (flag & (1UL << index)) > 0;
}

pure bool AreAnyBitsOn(u8  flag, u8 mask) { 
    return (flag & mask) > 0; 
}

pure bool AreAnyBitsOn(u16 flag, u16 mask) { 
    return (flag & mask) > 0; 
}

pure bool AreAnyBitsOn(u32 flag, u32 mask) {
    return (flag & mask) > 0; 
}

pure bool AreAnyBitsOn(u64 flag, u64 mask) { 
    return (flag & mask) > 0;
}

pure bool AreAllBitsOn(u8 flag, u8 mask) {
    return flag == mask; 
}

pure bool AreAllBitsOn(u16 flag, u16 mask) { 
    return flag == mask;
}

pure bool AreAllBitsOn(u32 flag, u32 mask) { 
    return flag == mask;
}

pure bool AreAllBitsOn(u64 flag, u64 mask) { 
    return flag == mask; 
}

#endif 