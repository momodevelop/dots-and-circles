#include <stdio.h>
#include <stdlib.h>
#include "ryoji_common.cpp"
#include "ryoji_bitmanip.h"

#define Assert(x)


// TODO(Momo): Consider shifting this part to Ryoji?
struct render_command_header {
    u32 Type;
    render_command_header* Next;
    void* Entry;
};

struct render_commands {
    // NOTE(Momo): Linked List of commands
    render_command_header* Head;
    render_command_header* Tail ;
    
    // NOTE(Momo): Memory for linked list above
    u8* Memory;
    u8* MemoryCurrent;
    u32 MemorySize;
    
    // TODO(Momo): Camera transforms?
    // TODO(Momo): Sorting?
};

static inline void
Init(render_commands* Commands, void* Memory, u32 MemorySize) {
    Commands->Head = nullptr;
    Commands->Tail = nullptr;
    Commands->Memory = (u8*)Memory;
    Commands->MemorySize = MemorySize;
    Commands->MemoryCurrent = (u8*)Memory;
}

static inline void*
Allocate(render_commands* Commands, u32 Size, u8 Alignment) {
    Assert(Size && Alignment);
    u8 Adjust = AlignForwardDiff(Commands->Memory, Alignment);
    
    u8* End = Commands->Memory + Commands->MemorySize;
    if (Commands->MemoryCurrent + Size + Adjust > End) {
        return nullptr; 
    }
    u8* Result = Commands->MemoryCurrent;
    Commands->MemoryCurrent += Size;
    
    return Result;
}


static inline void*
Push(render_commands* Commands, u32 Type, u32 Size, u8 Alignment) 
{
    // NOTE(Momo): Allocate header first
    render_command_header* Header = (render_command_header*)Allocate(Commands, 
                                                                     sizeof(render_command_header), 
                                                                     alignof(render_command_header));
    Assert(Header);
    Header->Type = Type;
    Header->Next = nullptr;
    
    if( Commands->Tail != nullptr ) {
        render_command_header* PrevHeader = (render_command_header*)Commands->Tail;
        PrevHeader->Next = Header;
    }
    Commands->Tail = Header;
    
    if (Commands->Head == nullptr) {
        Commands->Head = Commands->Tail;
    }
    
    // NOTE(Momo): Then allocate 
    Header->Entry = (u8*)Allocate(Commands, Size, Alignment);
    Assert(Header->Entry);
    return Header->Entry;
}


// NOTE(Momo): From here it's game related
enum  render_command_type {
    RenderCommandType_render_command_entry_clear,
    RenderCommandType_render_command_entry_textured_quad,
    RenderCommandType_render_command_entry_colored_quad,
};

struct render_command_entry_clear {
    u8 A, R, G, B;
};

struct render_command_entry_textured_quad {
    u32 TextureHandle; 
};

struct render_command_entry_colored_quad {
    u8 Alpha, Red, Green, Blue;
};

#define PushCommand(Commands, Type) (Type*)Push(Commands, RenderCommandType_##Type, sizeof(Type), alignof(Type))

int main() {
    void* Memory = malloc(1024);
    
    render_commands Commands;
    Init(&Commands, Memory, 1024);
    
    
    auto foo1 = PushCommand(&Commands, render_command_entry_clear);
    foo1->A = foo1->R = foo1->B = foo1->G = 125;
    
    auto foo2 = PushCommand(&Commands, render_command_entry_colored_quad);
    foo2->Alpha = foo2->Red = foo2->Blue = foo2->Green = 255;
    
    auto foo3 = PushCommand(&Commands, render_command_entry_textured_quad);
    foo3->TextureHandle = 1337;
    
    auto foo4 = PushCommand(&Commands, render_command_entry_textured_quad);
    foo4->TextureHandle = 1024;
    
    auto foo5 = PushCommand(&Commands, render_command_entry_clear);
    foo5->A = foo5->B = 123;
    
    for (render_command_header* Itr = (render_command_header*)Commands.Head; 
         Itr != nullptr; 
         Itr = Itr->Next) {
        
        render_command_header* Header = (render_command_header*)Itr;
        switch(Header->Type) {
            case RenderCommandType_render_command_entry_clear: {
                auto Entry = (render_command_entry_clear*)Header->Entry;
                printf("Clear! %d %d %d %d\n", Entry->A, Entry->R, Entry->G, Entry->B);
            } break;
            case RenderCommandType_render_command_entry_textured_quad: {
                auto Entry = (render_command_entry_textured_quad*)Header->Entry;
                printf("Quads! %d\n", Entry->TextureHandle);
            } break;
            case RenderCommandType_render_command_entry_colored_quad: {
                auto Entry = (render_command_entry_colored_quad*)Header->Entry;
                printf("Color Quads!  %d %d %d %d\n", Entry->Alpha, Entry->Red, Entry->Green, Entry->Blue);
            } break;
        }
        
    }
    free(Memory);
    return 0;
}

