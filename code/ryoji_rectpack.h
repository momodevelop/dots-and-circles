#ifndef RYY_RECTPACK_H
#define RYY_RECTPACK_H
#include "ryoji.h"


/* TODO: How to use:
*/

enum ryyrp_sort_type {
    RyyrpSort_Width,
    RyyrpSort_Height,
    RyyrpSort_Area,
    RyyrpSort_Perimeter,
    RyyrpSort_Pathological,
    RyyrpSort_BiggerSide,
};

struct ryyrp_node {
    u32 X, Y, W, H;
};

struct ryyrp_rect {
    // NOTE(Momo): Input
    u32 W, H; 
    
    // NOTE(Momo): For user's use
    void* UserData;
    
    // NOTE(Momo): Output
    u32 X, Y;
    
    
};

struct ryyrp_context {
    u32 Width;
    u32 Height;
    ryyrp_node* Nodes;
    usize NodeCount;
};

static inline void
ryyrp_Init(ryyrp_context* Context, u32 Width, u32 Height, ryyrp_node* Nodes, usize NodeCount) 
{
    Context->Width = Width;
    Context->Height = Height;
    Context->Nodes = Nodes;
    Context->NodeCount = NodeCount;
}



static inline i32 
ryyrp__SortWidth(const void* Lhs, const void* Rhs) {
    auto L = (*(ryyrp_rect*)(Lhs));
    auto R = (*(ryyrp_rect*)(Rhs));
    return R.W - L.W ;
}


static inline i32 
ryyrp__SortHeight(const void* Lhs, const void* Rhs) {
    auto L = (*(ryyrp_rect*)(Lhs));
    auto R = (*(ryyrp_rect*)(Rhs));
    return R.H - L.H;
}

static inline i32 
ryyrp__SortPerimeter(const void* Lhs, const void* Rhs) {
    auto L = (*(ryyrp_rect*)(Lhs));
    auto R = (*(ryyrp_rect*)(Rhs));
    auto LhsPerimeter = L.W + L.H;
    auto RhsPerimeter = R.W + R.H;
    return RhsPerimeter - LhsPerimeter;
}

static inline i32 
ryyrp__SortArea(const void* Lhs, const void* Rhs) {
    auto L = (*(ryyrp_rect*)(Lhs));
    auto R = (*(ryyrp_rect*)(Rhs));
    auto LhsArea = L.W * L.H;
    auto RhsArea = R.W * R.H;
    return  RhsArea - LhsArea;
}

static inline i32
ryyrp__SortBiggerSide(const void* Lhs, const void* Rhs) {
    auto L = (*(ryyrp_rect*)(Lhs));
    auto R = (*(ryyrp_rect*)(Rhs));
    auto LhsBiggerSide = Maximum(L.W, L.H);
    auto RhsBiggerSide = Maximum(R.W, R.H);
    return RhsBiggerSide - LhsBiggerSide;
}

static inline i32
ryyrp__SortPathological(const void* Lhs, const void* Rhs) {
    auto L = (*(ryyrp_rect*)(Lhs));
    auto R = (*(ryyrp_rect*)(Rhs));
    auto LhsMultipler = Maximum(L.W, L.H)/Minimum(L.W, L.H) * L.W * L.H;
    auto RhsMultipler = Maximum(R.W, R.H)/Minimum(R.W, R.H) * R.W * R.H;
    return RhsMultipler - LhsMultipler;
}


static inline void 
ryyrp__Sort(ryyrp_rect* Rects, usize RectCount, ryyrp_sort_type SortType) {
    switch(SortType) {
        case RyyrpSort_Width: {
            qsort(Rects, RectCount, sizeof(ryyrp_rect), ryyrp__SortWidth);
        } break;
        case RyyrpSort_Height: {
            qsort(Rects, RectCount, sizeof(ryyrp_rect), ryyrp__SortHeight);
        } break;
        case RyyrpSort_Area: {
            qsort(Rects, RectCount, sizeof(ryyrp_rect), ryyrp__SortArea);
        } break;
        case RyyrpSort_Perimeter: {
            qsort(Rects, RectCount, sizeof(ryyrp_rect), ryyrp__SortPerimeter);
        } break;
        case RyyrpSort_Pathological: {
            qsort(Rects, RectCount, sizeof(ryyrp_rect), ryyrp__SortPathological);
        } break;
        default: {
            Assert(false);
        };
    }
}



// NOTE(Momo): Rects WILL be sorted after this function
static inline b32
ryyrp_Pack(ryyrp_context* Context, ryyrp_rect* Rects, usize RectCount, ryyrp_sort_type SortType = RyyrpSort_Area) 
{
    ryyrp__Sort(Rects, RectCount, SortType);
    
    usize CurrentNodeCount = 0;
    Context->Nodes[CurrentNodeCount++] = { 0, 0, Context->Width, Context->Height };
    
    for (u32 i = 0; i < RectCount; ++i ) {
        auto Rect = Rects[i];
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        usize ChosenSpaceIndex = CurrentNodeCount;
        {
            for (usize  j = 0; j < ChosenSpaceIndex ; ++j ) {
                usize Index = ChosenSpaceIndex - j - 1;
                ryyrp_node Space = Context->Nodes[Index];
                // NOTE(Momo): Check if the image fits
                if (Rect.W <= Space.W && Rect.H <= Space.H) {
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
        ryyrp_node ChosenSpace = Context->Nodes[ChosenSpaceIndex];
        if (CurrentNodeCount > 0) {
            Context->Nodes[ChosenSpaceIndex] = Context->Nodes[CurrentNodeCount-1];
            --CurrentNodeCount;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpace.W != Rect.W && ChosenSpace.H == Rect.H) {
            // Split right
            ryyrp_node SplitSpaceRight = {
                ChosenSpace.X + Rect.W,
                ChosenSpace.Y,
                ChosenSpace.W - Rect.W,
                ChosenSpace.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpace.W == Rect.W && ChosenSpace.H != Rect.H) {
            // Split down
            ryyrp_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Rect.H,
                ChosenSpace.W,
                ChosenSpace.H - Rect.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpace.W != Rect.W && ChosenSpace.H != Rect.H) {
            // Split right
            ryyrp_node SplitSpaceRight = {
                ChosenSpace.X + Rect.W,
                ChosenSpace.Y,
                ChosenSpace.W - Rect.W,
                Rect.H,
            };
            
            // Split down
            ryyrp_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Rect.H,
                ChosenSpace.W,
                ChosenSpace.H - Rect.H,
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
        
        // NOTE(Momo): Translate the rect
        Rects[i].X = ChosenSpace.X;
        Rects[i].Y = ChosenSpace.Y;
    }
    
    return true;
}

#endif //RYOJI_RECTPACK_H
