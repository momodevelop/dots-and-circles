#include <stdio.h>
#include "../../code/momo.h"


// NOTE(Momo): Number is stored in the reverse order
// That is, the number '12345' is stored as '5432100000'.
struct big_int {
    u8* Data;
    u32 Count;
    u32 Places;
};


//~NOTE(Momo): Forward Iterator
struct big_int_forward_itr {
    big_int* BigInt;
    u32 Index;
    
};

static inline big_int_forward_itr
BigInt_ForwardItrBegin(big_int* B) {
    return { B, 0 };
}

static inline big_int_forward_itr
BigInt_ForwardItrEnd(big_int* B) {
    return { B, B->Places };
}

static inline b8
operator!=(big_int_forward_itr Lhs, big_int_forward_itr Rhs) {
    return Lhs.Index != Rhs.Index;
}

static inline big_int_forward_itr&
operator++(big_int_forward_itr& Itr) {
    ++Itr.Index;
    return Itr;
}

static inline u8&
operator*(big_int_forward_itr& Itr) {
    return Itr.BigInt->Data[Itr.Index];
}

//~NOTE(Momo): Reverse Iterator
struct big_int_reverse_itr {
    big_int* BigInt;
    u32 Index;
};

static inline big_int_reverse_itr
BigInt_ReverseItrBegin(big_int* B) {
    return { B, 0 };
}

static inline big_int_reverse_itr
BigInt_ReverseItrEnd(big_int* B) {
    return { B, B->Places };
}

static inline b8
operator!=(big_int_reverse_itr Lhs, big_int_reverse_itr Rhs) {
    return Lhs.Index != Rhs.Index;
}

static inline big_int_reverse_itr&
operator++(big_int_reverse_itr& Itr) {
    ++Itr.Index;
    return Itr;
}

static inline u8&
operator*(big_int_reverse_itr& Itr) {
    u32 ActualIndex = Itr.BigInt->Places - Itr.Index - 1;
    return Itr.BigInt->Data[ActualIndex];
}


//~ NOTE(Momo): Functions
static inline void
BigInt_Reset(big_int * B) {
    for (u32 I = 0; I < B->Count; ++I) {
        B->Data[I] = 0;
    }
    B->Places = 1;
}

static inline b8
BigInt_Init(big_int* B, u8* Buffer, u32 Count) {
    if (!Buffer || Count == 0) {
        return false;
    }
    B->Data = Buffer;
    B->Count = Count;
    
    BigInt_Reset(B);
    return true;
}

static inline b8
BigInt_New(big_int* B, arena* Arena, u32 Count) {
    u8* Buffer = Arena_PushArray<u8>(Arena, Count);
    return BigInt_Init(B, Buffer, Count);
}

static inline void
BigInt_Add(big_int* B, u32 Value) {
    // NOTE(Momo): For each place, add
    u32 Index = 0;
    u8 Carry = 0;
    while (Value > 0) {
        Assert(Index < B->Count);
        u8 ExtractedValue = (u8)(Value % 10);
        u8 Result = ExtractedValue + Carry + B->Data[Index];
        if (Result >= 10) {
            Carry = 1;
            Result -= 10;
        }
        else {
            Carry = 0;
        }
        B->Data[Index] = Result; 
        Value /= 10;
        ++Index;
        
    }
    
    while(Carry > 0) {
        Assert(Index < B->Count);
        u8 Result = B->Data[Index] + Carry;
        if (Result >= 10) {
            Carry = 1;
            Result -= 10;
        }
        else {
            Carry = 0;
        }
        B->Data[Index] = Result;
        ++Index;
    }
    
    if (Index > B->Places) {
        B->Places = Index;
    }
}

static inline big_int&
operator+=(big_int& Lhs, u32 Rhs) {
    BigInt_Add(&Lhs, Rhs);
    return Lhs;
}

int main() {
    u8 A[10];
    big_int _L = {};
    big_int* L = &_L;
    
    BigInt_Init(L, A, 100);
    BigInt_Add(L, 12345);
    _L += 12345;
    
    for(auto Beg = BigInt_ForwardItrBegin(L); Beg != BigInt_ForwardItrEnd(L); ++Beg) {
        printf("%d", (*Beg));
    }
    
    printf("\n");
    
    for(auto Beg = BigInt_ReverseItrBegin(L); Beg != BigInt_ReverseItrEnd(L); ++Beg) {
        printf("%d", (*Beg));
    }
    
    printf("\n");
    
    
    printf("Places: %d", L->Places);
    printf("\n");
}