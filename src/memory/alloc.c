// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

/** @file
 *  @ingroup Memory_allocation
 *  Internal wrappers for `malloc`, `realloc`, and `free` */

#include <stdio.h>
#include <stdlib.h>
#include "memory/alloc.h"

/** @ingroup Memory_allocation
 *  @{ */

void* srpMalloc(unsigned long size)
{
	void* p = malloc(size);
	if (p != NULL)
		return p;
	fprintf(stderr, "libsrp: malloc returned NULL (out of memory), aborting...\n");
	abort();
}

void* srpRealloc(void* p, unsigned long size)
{
	void* pNew = realloc(p, size);
	if (pNew != NULL)
		return pNew;
	fprintf(stderr, "libsrp: realloc returned NULL (out of memory), aborting...\n");
	abort();
}

/** @} */  // ingroup Memory_allocation
