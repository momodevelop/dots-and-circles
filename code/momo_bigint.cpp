static inline void
BigInt_SetZero(BigInt* b) {
    for (u32 i = 0; i < b->cap; ++i) {
        b->data[i] = 0;
    }
    b->count = 1;
}

static inline b8
BigInt_Init(BigInt* b, u8* buffer, u32 length) {
    if (!buffer || length == 0) {
        return false;
    }
    b->data = buffer;
    b->cap = length;
    
    BigInt_SetZero(b);
    return true;
}


static inline void
BigInt_SetMax(BigInt* b) {
    for(u32 i = 0; i < b->cap; ++i) {
        b->data[i] = 9;
    }
    b->count = b->cap;
}

static inline b8
BigInt_Alloc(BigInt* b, Arena* arena, u32 length) {
    u8* buffer = Arena_PushArray<u8>(arena, length);
    return BigInt_Init(b, buffer, length);
}

static inline void
BigInt_Add(BigInt* b, u32 value) {
    // NOTE(Momo): For each place, add
    u32 index = 0;
    u8 carry = 0;
    while (value > 0) {
        if (index >= b->cap) {
            BigInt_SetMax(b);
            return;
        }
        u8 extracted_value = (u8)(value % 10);
        u8 result = extracted_value + carry + b->data[index];
        if (result >= 10) {
            carry = 1;
            result -= 10;
        }
        else {
            carry = 0;
        }
        b->data[index] = result; 
        value /= 10;
        ++index;
        
    }
    
    while(carry > 0) {
        if (index >= b->cap) {
            BigInt_SetMax(b);
            return;
        }
        u8 result = b->data[index] + carry;
        if (result >= 10) {
            carry = 1;
            result -= 10;
        }
        else {
            carry = 0;
        }
        b->data[index] = result;
        ++index;
    }
    
    if (index > b->count) {
        b->count = index;
    }
}

static inline void
BigInt_Set(BigInt* b, u32 value) {
    // NOTE(Momo): For each place, add
    u32 index = 0;
    u8 carry = 0;
    while (value > 0) {
        if (index >= b->cap) {
            BigInt_SetMax(b);
            return;
        }
        
        b->data[index] = (u8)(value % 10); 
        value /= 10;
        ++index;
    }
    
    if (index > b->count) {
        b->count = index;
    }
}
