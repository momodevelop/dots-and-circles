#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

#include <stdlib.h>
#include "mm_core.h"


enum ap_sort_type {
    MmrpSort_Width,
    MmrpSort_Height,
    MmrpSort_Area,
    MmrpSort_Perimeter,
    MmrpSort_Pathological,
    MmrpSort_BiggerSide,
};

struct ap_node {
    u32 X, Y, W, H;
};

struct ap_aabb {
    // NOTE(Momo): Input
    u32 W, H; 
    
    // NOTE(Momo): For user's use
    void* UserData;
    
    // NOTE(Momo): Output
    u32 X, Y;
};

struct ap_context {
    u32 Width;
    u32 Height;
    ap_node* Nodes;
    usize NodeCount;
};

static inline ap_context
ap_CreateAabbPacker(u32 Width, u32 Height, ap_node* Nodes, usize NodeCount) 
{
    ap_context Ret = {};
    Ret.Width = Width;
    Ret.Height = Height;
    Ret.Nodes = Nodes;
    Ret.NodeCount = NodeCount;

    return Ret;
}

static inline i32 
ap__SortWidth(const void* Lhs, const void* Rhs) {
    auto L = (*(ap_aabb*)(Lhs));
    auto R = (*(ap_aabb*)(Rhs));
    return R.W - L.W ;
}


static inline i32 
ap__SortHeight(const void* Lhs, const void* Rhs) {
    auto L = (*(ap_aabb*)(Lhs));
    auto R = (*(ap_aabb*)(Rhs));
    return R.H - L.H;
}

static inline i32 
ap__SortPerimeter(const void* Lhs, const void* Rhs) {
    auto L = (*(ap_aabb*)(Lhs));
    auto R = (*(ap_aabb*)(Rhs));
    auto LhsPerimeter = L.W + L.H;
    auto RhsPerimeter = R.W + R.H;
    return RhsPerimeter - LhsPerimeter;
}

static inline i32 
ap__SortArea(const void* Lhs, const void* Rhs) {
    auto L = (*(ap_aabb*)(Lhs));
    auto R = (*(ap_aabb*)(Rhs));
    auto LhsArea = L.W * L.H;
    auto RhsArea = R.W * R.H;
    return  RhsArea - LhsArea;
}

static inline i32
ap__SortBiggerSide(const void* Lhs, const void* Rhs) {
    auto L = (*(ap_aabb*)(Lhs));
    auto R = (*(ap_aabb*)(Rhs));
    auto LhsBiggerSide = Maximum(L.W, L.H);
    auto RhsBiggerSide = Maximum(R.W, R.H);
    return RhsBiggerSide - LhsBiggerSide;
}

static inline i32
ap__SortPathological(const void* Lhs, const void* Rhs) {
    auto L = (*(ap_aabb*)(Lhs));
    auto R = (*(ap_aabb*)(Rhs));
    auto LhsMultipler = Maximum(L.W, L.H)/Minimum(L.W, L.H) * L.W * L.H;
    auto RhsMultipler = Maximum(R.W, R.H)/Minimum(R.W, R.H) * R.W * R.H;
    return RhsMultipler - LhsMultipler;
}


static inline void 
ap__Sort(ap_aabb* Aabbs, usize AabbCount, ap_sort_type SortType) {
    switch(SortType) {
        case MmrpSort_Width: {
            qsort(Aabbs, AabbCount, sizeof(ap_aabb), ap__SortWidth);
        } break;
        case MmrpSort_Height: {
            qsort(Aabbs, AabbCount, sizeof(ap_aabb), ap__SortHeight);
        } break;
        case MmrpSort_Area: {
            qsort(Aabbs, AabbCount, sizeof(ap_aabb), ap__SortArea);
        } break;
        case MmrpSort_Perimeter: {
            qsort(Aabbs, AabbCount, sizeof(ap_aabb), ap__SortPerimeter);
        } break;
        case MmrpSort_Pathological: {
            qsort(Aabbs, AabbCount, sizeof(ap_aabb), ap__SortPathological);
        } break;
        default: {
            Assert(false);
        };
    }
}



// NOTE(Momo): Aabbs WILL be sorted after this function
static inline b32
ap_Pack(ap_context* Context, ap_aabb* Aabbs, usize AabbCount, ap_sort_type SortType = MmrpSort_Area) 
{
    ap__Sort(Aabbs, AabbCount, SortType);
    
    usize CurrentNodeCount = 0;
    Context->Nodes[CurrentNodeCount++] = { 0, 0, Context->Width, Context->Height };
    
    for (u32 i = 0; i < AabbCount; ++i ) {
        auto Aabb = Aabbs[i];
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        usize ChosenSpaceIndex = CurrentNodeCount;
        {
            for (usize  j = 0; j < ChosenSpaceIndex ; ++j ) {
                usize Index = ChosenSpaceIndex - j - 1;
                ap_node Space = Context->Nodes[Index];
                // NOTE(Momo): Check if the image fits
                if (Aabb.W <= Space.W && Aabb.H <= Space.H) {
                    ChosenSpaceIndex = Index;
                    break;
                }
            }
        }
        
        
        // NOTE(Momo): If an empty space that can fit is found, we remove that space and split.
        if (ChosenSpaceIndex == CurrentNodeCount) {
            return false;
        }
        
        // NOTE(Momo): Swap and pop the chosen space
        ap_node ChosenSpace = Context->Nodes[ChosenSpaceIndex];
        if (CurrentNodeCount > 0) {
            Context->Nodes[ChosenSpaceIndex] = Context->Nodes[CurrentNodeCount-1];
            --CurrentNodeCount;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpace.W != Aabb.W && ChosenSpace.H == Aabb.H) {
            // Split right
            ap_node SplitSpaceRight = {
                ChosenSpace.X + Aabb.W,
                ChosenSpace.Y,
                ChosenSpace.W - Aabb.W,
                ChosenSpace.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpace.W == Aabb.W && ChosenSpace.H != Aabb.H) {
            // Split down
            ap_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Aabb.H,
                ChosenSpace.W,
                ChosenSpace.H - Aabb.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpace.W != Aabb.W && ChosenSpace.H != Aabb.H) {
            // Split right
            ap_node SplitSpaceRight = {
                ChosenSpace.X + Aabb.W,
                ChosenSpace.Y,
                ChosenSpace.W - Aabb.W,
                Aabb.H,
            };
            
            // Split down
            ap_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Aabb.H,
                ChosenSpace.W,
                ChosenSpace.H - Aabb.H,
            };
            
            // Choose to insert the bigger one first before the smaller one
            u32 RightArea = SplitSpaceRight.W * SplitSpaceRight.H;
            u32 DownArea = SplitSpaceDown.W * SplitSpaceDown.H;
            if (RightArea > DownArea) {
                Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
                Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
            }
            else {
                Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
                Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
            }
            
        }
        
        // NOTE(Momo): Translate the Aabb
        Aabbs[i].X = ChosenSpace.X;
        Aabbs[i].Y = ChosenSpace.Y;
    }
    
    return true;
}

#endif //RYOJI_Aabb_packer_H
