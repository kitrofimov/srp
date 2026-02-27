// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Memory_allocation
 *  SRPArena and related functions */

#pragma once

#include <stddef.h>
#include "srp/context.h"

/** @ingroup Memory_allocation
 *  @{ */

/** Default capacity of SRPArena */
#define SRP_DEFAULT_ARENA_CAPACITY (1024 * 1024)  // 1 MiB

/** Represents a memory block in the SRPArena */
typedef struct SRPArenaBlock {
    struct SRPArenaBlock* next;  /**< Pointer to the next block */
    size_t capacity;             /**< Total allocated size of this block */
    size_t used;                 /**< How many bytes are used */
    unsigned char data[];        /**< Pointer to the data */
} SRPArenaBlock;

/** Blockchain-based arena allocator */
typedef struct SRPArena {
    SRPArenaBlock* head;     /**< Pointer to the first block */
    SRPArenaBlock* current;  /**< Pointer to the currently-being-filled block */
    size_t pageSize;         /**< Default capacity for newly created blocks */
} SRPArena;

/** Create a new arena with given capacity. Should be freed with freeArena()
 *  @param[in] capacity The capacity of the arena in bytes
 *  @return Pointer to the newly created arena */
SRPArena* newArena(size_t capacity);

/** Free the arena and all memory allocated within it
 *  @param[in] this Pointer to the arena, as returned by newArena() */
void freeArena(SRPArena* this);

/** Allocate a block of memory in the arena
 *  @param[in] this Pointer to the arena
 *  @param[in] size Size of the block to allocate in bytes
 *  @return Pointer to the allocated block */
void* arenaAlloc(SRPArena* this, size_t size);

/** Allocate a block of memory in the arena, initializing the memory to zero
 *  @param[in] this Pointer to the arena
 *  @param[in] size Size of the block to allocate in bytes
 *  @return Pointer to the allocated block */
void* arenaCalloc(SRPArena* this, size_t size);

/** Reset the arena, freeing all allocated memory. Does not free the arena itself.
 *  @param[in] this Pointer to the arena */
void arenaReset(SRPArena* this);

// Macros to manipulate context-stored arena
#define ARENA_ALLOC(size) (arenaAlloc(srpContext.arena, (size)))
#define ARENA_CALLOC(size) (arenaCalloc(srpContext.arena, (size)))
#define ARENA_RESET() (arenaReset(srpContext.arena))

/** @} */  // ingroup Memory_allocation
