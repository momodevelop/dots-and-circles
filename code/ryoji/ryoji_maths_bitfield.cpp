#ifndef __RYOJI_MATHS_BITFIELDS__
#define __RYOJI_MATHS_BITFIELDS__

pure u8 mask(u8 flag, u8 mask) { 
    return flag | mask; 
}

pure u16 mask(u16 flag, u16 mask) { 
    return flag | mask;
}

pure u32 mask(u32 flag, u32 mask) { 
    return flag | mask; 
}

pure u64 mask(u64 flag, u64 mask) {
    return flag | mask; 
}

pure u8 unmask(u8 flag, u8 mask) 	{ 
    return flag & ~mask; 
}

pure u16 unmask(u16 flag, u16 mask)  { 
    return flag & ~mask; 
}

pure u32 unmask(u32 flag, u32 mask)  {
    return flag & ~mask; 
}

pure u64 unmask(u64 flag, u64 mask)  {
    return flag & ~mask; 
}

pure u8 set(u8 flag, usize index, bool val)  {
    Assert((sizeof(u8) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u16 set(u16 flag, usize index, bool val)  {
    Assert((sizeof(u16) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u32 set(u32 flag, usize index, bool val)  {
    Assert((sizeof(u32) * 8) > (index - 1));
    return (flag & ~(1U << index)) | (val << index);
}
pure u64 set(u64 flag, usize index, bool val)  {
    Assert((sizeof(u64) * 8) > (index - 1));
    return (flag & ~(1ULL << index)) | (val << index);
}

pure u8 get(u8 flag, usize index, bool val)  {
    Assert((sizeof(u8) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u16 get(u16 flag, usize index, bool val)  {
    Assert((sizeof(u16) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u32 get(u32 flag, usize index, bool val)  {
    Assert((sizeof(u32) * 8) > (index - 1));
    return (flag & (1U << index)) > 0;
}
pure u64 get(u64 flag, usize index, bool val)  {
    Assert((sizeof(u64) * 8) > (index - 1));
    return (flag & (1UL << index)) > 0;
}

pure bool any(u8  flag, u8 mask) { 
    return (flag & mask) > 0; 
}

pure bool any(u16 flag, u16 mask) { 
    return (flag & mask) > 0; 
}

pure bool any(u32 flag, u32 mask) {
    return (flag & mask) > 0; 
}

pure bool any(u64 flag, u64 mask) { 
    return (flag & mask) > 0;
}

pure bool all(u8 flag, u8 mask) {
    return flag == mask; 
}

pure bool all(u16 flag, u16 mask) { 
    return flag == mask;
}

pure bool all(u32 flag, u32 mask) { 
    return flag == mask;
}

pure bool all(u64 flag, u64 mask) { 
    return flag == mask; 
}


#endif