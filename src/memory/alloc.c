// Software Rendering Pipeline (SRP) library
// Licensed under GNU GPLv3

#include <stdio.h>
#include <stdlib.h>
#include "memory/alloc.h"

void* srpMalloc(unsigned long size)
{
	void* p = malloc(size);
	if (p != NULL)
		return p;
	fprintf(stderr, "libsrp: malloc returned NULL (out of memory), aborting...\n");
	abort();
}

void srpFree(void* p)
{
	free(p);
}

void* srpRealloc(void* p, unsigned long size)
{
	void* pNew = realloc(p, size);
	if (pNew != NULL)
		return pNew;
	fprintf(stderr, "libsrp: realloc returned NULL (out of memory), aborting...\n");
	abort();
}

