// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Memory_allocation
 *  Declaration of internal wrappers for `malloc`, `realloc`, and `free` */

#pragma once

/** @ingroup Memory_allocation
 *  @{ */

/** `malloc` wrapper. Checks its return value and calls abort() if it has
 *  returned NULL. @see SRP_MALLOC */
void* srpMalloc(unsigned long size);

/** `realloc` wrapper. Checks its return value and calls abort() if it has
 *  returned NULL. @see SRP_REALLOC */
void* srpRealloc(void* p, unsigned long size);

/** @} */  // ingroup Memory_allocation
