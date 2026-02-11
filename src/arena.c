// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include "arena_p.h"
#include "defines.h"
#include "utils.h"

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_8_UP(x) (ALIGN_UP(x, 8))

/** @file
 *  Arena implementation */

/** @ingroup Arena_internal
 *  @{ */

/** Allocate new block
 *  @param[in] capacity Capacity of the block in bytes
 *  @return Pointer to the newly allocated block */
static SRPArenaBlock* newBlock(size_t capacity);

/** Determine the needed block size for a given requested size
 *  @param[in] arena Pointer to the arena
 *  @param[in] requested Requested size in bytes
 *  @return Needed block size in bytes */
static size_t neededBlockSize(SRPArena* arena, size_t requested);

/** @} */  // ingroup Arena_internal

static SRPArenaBlock* newBlock(size_t capacity)
{
    SRPArenaBlock* block = SRP_MALLOC(sizeof(SRPArenaBlock) + capacity);
    block->next = NULL;
    block->capacity = capacity;
    block->used = 0;
    return block;
}

SRPArena* newArena(size_t capacity)
{
    SRPArena* arena = SRP_MALLOC(sizeof(SRPArena));
    arena->pageSize = capacity < SRP_DEFAULT_ARENA_CAPACITY ? SRP_DEFAULT_ARENA_CAPACITY : capacity;
    arena->head = newBlock(arena->pageSize);
    arena->current = arena->head;
    return arena;
}

void freeArena(SRPArena* this)
{
    SRPArenaBlock* block = this->head;
    while (block) {
        SRPArenaBlock* next = block->next;
        SRP_FREE(block);
        block = next;
    }
    SRP_FREE(this);
}

static size_t neededBlockSize(SRPArena* this, size_t requested)
{
    size_t size = this->pageSize;
    if (requested > size)
        while (size < requested)
            size *= 2;
    return size;
}

void* arenaAlloc(SRPArena* this, size_t size)
{
    if (size == 0)
        return NULL;

    size_t aligned_used = ALIGN_8_UP(this->current->used);
    if (aligned_used + size > this->current->capacity) {
        size_t capacity = neededBlockSize(this, size);
        SRPArenaBlock* nb = newBlock(capacity);
        this->current->next = nb;
        this->current = nb;
        aligned_used = 0;
    }

    void* ptr = this->current->data + aligned_used;
    this->current->used = aligned_used + size;
    return ptr;
}

void arenaReset(SRPArena* this)
{
    size_t sum_used = this->head->used;
    SRPArenaBlock* block = this->head->next;
    while (block) {
        SRPArenaBlock* next = block->next;
        sum_used += block->used;
        SRP_FREE(block);
        block = next;
    }

    if (sum_used > this->pageSize) {
        this->pageSize = neededBlockSize(this, sum_used);
        SRP_FREE(this->head); 
        this->head = newBlock(this->pageSize);
        this->current = this->head;
    } else {
        this->head->next = NULL;
        this->head->used = 0;
        this->current = this->head;
    }
}
