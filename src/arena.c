// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "arena_p.h"
#include "defines.h"
#include "utils.h"

SRPArena* newArena(size_t capacity)
{
    SRPArena* arena = SRP_MALLOC(sizeof(SRPArena) + capacity);
    arena->buffer = INDEX_VOID_PTR(arena, sizeof(SRPArena), 1);
    arena->offset = 0;
    arena->capacity = capacity;
    return arena;
}

void* arenaAlloc(SRPArena* this, size_t size)
{
    // Not enough space in the arena
    if (this->offset + size > this->capacity)
    {
        size_t new_capacity = this->capacity;
        while (this->offset + size > new_capacity)
            new_capacity *= 2;
        this->buffer = SRP_REALLOC(this->buffer, new_capacity);
        this->capacity = new_capacity;
    }

    void* ptr = ADD_VOID_PTR(this->buffer, this->offset);
    this->offset += size;
    return ptr;
}

void arenaReset(SRPArena* this)
{
    this->offset = 0;
}

void freeArena(SRPArena* this)
{
    SRP_FREE(this->buffer);
    SRP_FREE(this);
}
