#ifndef __MM_AABB_PACKER__
#define __MM_AABB_PACKER__

// TODO(Momo): Very low priority, but we should rethink this
enum aabb_packer_sort_type {
    AabbPackerSortType_Width,
    AabbPackerSortType_Height,
    AabbPackerSortType_Area,
    AabbPackerSortType_Perimeter,
    AabbPackerSortType_Pathological,
    AabbPackerSortType_BiggerSide,
};

struct aabb_packer_node {
    u32 X, Y, W, H;
};

struct aabb_packer_rect {
    // NOTE(Momo): Input
    u32 W, H; 
    
    // NOTE(Momo): For user's use
    void* UserData;
    
    // NOTE(Momo): Output
    u32 X, Y;
};

struct aabb_packer {
    u32 Width;
    u32 Height;
    aabb_packer_node* Nodes;
    usize NodeCount;
};

static inline aabb_packer
AabbPacker_Create(u32 Width, 
                  u32 Height, 
                  aabb_packer_node* Nodes, 
                  usize NodeCount) 
{
    aabb_packer Ret = {};
    Ret.Width = Width;
    Ret.Height = Height;
    Ret.Nodes = Nodes;
    Ret.NodeCount = NodeCount;
    
    return Ret;
}

static inline b32 
__AabbPacker_SortWidth(aabb_packer_rect Lhs, 
                       aabb_packer_rect Rhs) {
    return Rhs.W > Lhs.W ;
}


static inline b32 
__AabbPacker_SortHeight(aabb_packer_rect Lhs,
                        aabb_packer_rect Rhs) 
{
    return Rhs.H > Lhs.H;
}

static inline b32 
__AabbPacker_SortPerimeter(aabb_packer_rect Lhs,
                           aabb_packer_rect Rhs)
{
    u32 LhsPerimeter = Lhs.W + Lhs.H;
    u32 RhsPerimeter = Rhs.W + Rhs.H;
    return RhsPerimeter > LhsPerimeter;
}

static inline b32
__AabbPacker_SortArea(aabb_packer_rect Lhs, 
                      aabb_packer_rect Rhs) 
{
    u32 LhsArea = Lhs.W * Lhs.H;
    u32 RhsArea = Rhs.W * Rhs.H;
    return  RhsArea > LhsArea;
}

static inline b32
__AabbPacker_SortBiggerSide(aabb_packer_rect Lhs,
                            aabb_packer_rect Rhs) 
{
    u32 LhsBiggerSide = MaxOf(Lhs.W, Lhs.H);
    u32 RhsBiggerSide = MaxOf(Rhs.W, Rhs.H);
    return RhsBiggerSide > LhsBiggerSide;
}

static inline b32
__AabbPacker_SortPathological(aabb_packer_rect Lhs, 
                              aabb_packer_rect Rhs) 
{
    u32 LhsMultipler = MaxOf(Lhs.W, Lhs.H)/MinOf(Lhs.W, Lhs.H) * Lhs.W * Lhs.H;
    u32 RhsMultipler = MaxOf(Rhs.W, Rhs.H)/MinOf(Rhs.W, Rhs.H) * Rhs.W * Rhs.H;
    return RhsMultipler > LhsMultipler;
}


static inline void 
__AabbPacker_Sort(aabb_packer_rect* Aabbs, 
                  usize AabbCount, 
                  aabb_packer_sort_type SortType) {
    switch(SortType) {
        case AabbPackerSortType_Width: {
            QuickSort(Aabbs, AabbCount, __AabbPacker_SortWidth);
        } break;
        case AabbPackerSortType_Height: {
            QuickSort(Aabbs, AabbCount, __AabbPacker_SortHeight);
        } break;
        case AabbPackerSortType_Area: {
            QuickSort(Aabbs, AabbCount, __AabbPacker_SortArea);
        } break;
        case AabbPackerSortType_Perimeter: {
            QuickSort(Aabbs, AabbCount, __AabbPacker_SortPerimeter);
        } break;
        case AabbPackerSortType_Pathological: {
            QuickSort(Aabbs, AabbCount, __AabbPacker_SortPathological);
        } break;
        case AabbPackerSortType_BiggerSide: {
            QuickSort(Aabbs, AabbCount, __AabbPacker_SortBiggerSide);
        } break;
        default: {
            Assert(false);
        };
    }
}



// NOTE(Momo): Aabbs WILL be sorted after this function
static inline b32
AabbPacker_Pack(aabb_packer* Context, 
                aabb_packer_rect* Aabbs, 
                usize AabbCount, 
                aabb_packer_sort_type SortType) 
{
    __AabbPacker_Sort(Aabbs, AabbCount, SortType);
    
    usize CurrentNodeCount = 0;
    Context->Nodes[CurrentNodeCount++] = { 0, 0, Context->Width, Context->Height };
    
    for (u32 i = 0; i < AabbCount; ++i ) {
        auto Aabb = Aabbs[i];
        
        // NOTE(Momo): Iterate the empty spaces backwards to find the best fit index
        usize ChosenSpaceIndex = CurrentNodeCount;
        {
            for (usize  j = 0; j < ChosenSpaceIndex ; ++j ) {
                usize Index = ChosenSpaceIndex - j - 1;
                aabb_packer_node Space = Context->Nodes[Index];
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
        aabb_packer_node ChosenSpace = Context->Nodes[ChosenSpaceIndex];
        if (CurrentNodeCount > 0) {
            Context->Nodes[ChosenSpaceIndex] = Context->Nodes[CurrentNodeCount-1];
            --CurrentNodeCount;
        }
        
        // NOTE(Momo): Split if not perfect fit
        if (ChosenSpace.W != Aabb.W && ChosenSpace.H == Aabb.H) {
            // Split right
            aabb_packer_node SplitSpaceRight = {
                ChosenSpace.X + Aabb.W,
                ChosenSpace.Y,
                ChosenSpace.W - Aabb.W,
                ChosenSpace.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceRight;
        }
        else if (ChosenSpace.W == Aabb.W && ChosenSpace.H != Aabb.H) {
            // Split down
            aabb_packer_node SplitSpaceDown = {
                ChosenSpace.X,
                ChosenSpace.Y + Aabb.H,
                ChosenSpace.W,
                ChosenSpace.H - Aabb.H
            };
            Context->Nodes[CurrentNodeCount++] = SplitSpaceDown;
        }
        else if (ChosenSpace.W != Aabb.W && ChosenSpace.H != Aabb.H) {
            // Split right
            aabb_packer_node SplitSpaceRight = {
                ChosenSpace.X + Aabb.W,
                ChosenSpace.Y,
                ChosenSpace.W - Aabb.W,
                Aabb.H,
            };
            
            // Split down
            aabb_packer_node SplitSpaceDown = {
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

#endif
