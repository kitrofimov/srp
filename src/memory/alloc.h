// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  Declarations of wrappers to `malloc`, `free` and `realloc` */

#pragma once

/** @ingroup Memory_allocation
 *  @{ */

/** `malloc` wrapper. Checks its return value and calls abort() if it has
 *  returned NULL. @see SRP_MALLOC */
void* srpMalloc(unsigned long size);

/** `free` wrapper. Practically useless, but made along with srpMalloc() and
 *  srpRealloc(). @see SRP_FREE */
void srpFree(void* p);

/** `realloc` wrapper. Checks its return value and calls abort() if it has
 *  returned NULL. @see SRP_REALLOC */
void* srpRealloc(void* p, unsigned long size);

/** @} */  // ingroup Memory_allocation

