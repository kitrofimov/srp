// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  SRPArena and related functions */

#pragma once

#include <stddef.h>

/** @ingroup Arena
 *  @{ */

#define SRP_DEFAULT_ARENA_CAPACITY (1024 * 1024)  // 1 MiB

typedef struct SRPArena
{
    void* buffer;
    size_t offset;
    size_t capacity;
} SRPArena;

/** Create a new arena with given capacity. Should be freed with freeArena()
 *  @param[in] capacity The capacity of the arena in bytes
 *  @return Pointer to the newly created arena */
SRPArena* newArena(size_t capacity);

/** Allocate a block of memory in the arena
 *  @param[in] this Pointer to the arena
 *  @param[in] size Size of the block to allocate in bytes
 *  @return Pointer to the allocated block */
void* arenaAlloc(SRPArena* this, size_t size);

/** Reset the arena, freeing all allocated memory. Does not free the arena itself.
 *  @param[in] this Pointer to the arena */
void arenaReset(SRPArena* this);

/** Free the arena and all memory allocated within it
 *  @param[in] this Pointer to the arena, as returned by newArena() */
void freeArena(SRPArena* this);

/** @} */  // defgroup Arena