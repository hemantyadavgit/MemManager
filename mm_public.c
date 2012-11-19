/***********************
Author: Hemant Yadav
***********************/
#include "mm_public.h"

/* Return usec */
double comp_time (struct timeval times, struct timeval timee)
{

  double elap = 0.0;

  if (timee.tv_sec > times.tv_sec) {
    elap += (((double)(timee.tv_sec - times.tv_sec -1))*1000000.0);
    elap += timee.tv_usec + (1000000-times.tv_usec);
  }
  else {
    elap = timee.tv_usec - times.tv_usec;
  }
  return ((unsigned long)(elap));

}

/*
	Splices pchunk from free list and updates manager
	If pchunk is first in the free list, free list set to pchunk's
	next, which may be null.	
*/
#define SPLICEOUT(MM,pchunk) { 					\
	if (pchunk->FD != NULL) pchunk->FD->BK = pchunk->BK;	\
	if (pchunk->BK != NULL) pchunk->BK->FD = pchunk->FD;	\
	if (MM->freelist == pchunk) MM->freelist =  pchunk->FD;	\
}

/* 
	Splices pchunk into free list
*/
#define SPLICEIN(MM,pchunk) {						\
	freechunk * node = MM->freelist;				\
	if (node == NULL) {						\
		MM->freelist = pchunk;					\
		pchunk->FD = NULL;					\
		pchunk->BK = NULL;					\
	} else {							\
		while(node->SZ < pchunk->SZ && node->FD != NULL)	\
		{							\
			printf("node: 0x%08x; fd: 0x%08x\n", node, node->FD);\
			node = node->FD;				\
		}							\
		if (pchunk->SZ <= node->SZ) {				\
			pchunk->BK = node->BK;				\
			pchunk->FD = node;				\
			node->BK = pchunk;				\
			if (pchunk->BK != NULL) pchunk->BK->FD = pchunk;\
			else MM->freelist = pchunk;			\
		} else {						\
			node->FD = pchunk;				\
			pchunk->BK = node;				\
			pchunk->FD = NULL;				\
		}							\
}	}
			
/*
	Forward-consolidates pair of chunks into one big free chunk and updates next chunk's size info
*/
#define CONSOLIDATE(MM,lchunk,rchunk) {					\
	lchunk->SZ += rchunk->SZ;					\
	lchunk->FD = rchunk->FD;					\
	chunk * nextchunk = (chunk *)((void *)lchunk+lchunk->SZ);	\
	if (nextchunk != (chunk *)(MM->front + MM->size))		\
		nextchunk->PREVSZ = lchunk->SZ;				\
}

int  mm_init (mm_t *MM, int tsz)
{
	if (tsz < sizeof(freechunk))		// minimum size is to hold one free chunk
		return -1;			
	if ((MM->front = malloc(tsz)) == NULL)	// fail
		return -1;
	MM->size = tsz;
	MM->freelist = (freechunk *)MM->front;	// make first node
	MM->freelist->PREVSZ = 0;
	MM->freelist->SZ = MM->size;
	MM->freelist->FD = NULL;		// no next node
	MM->freelist->BK = NULL;		// no prev node
	return 0;
}

void* mm_get (mm_t *MM, int size) 
{
	freechunk * free = MM->freelist;		// grab first (smallest) free chunk
	if (size < sizeof(freechunk)-sizeof(chunk))	// minimum chunk size: gives minimum future header space for larger free chunk header
		size = sizeof(freechunk)-sizeof(chunk);
	while (free != NULL && free->SZ-sizeof(chunk) < size)	// search for tightest-fitting node
		free = free->FD;
	if (free == NULL)	// no node available
	{
		fprintf(stderr, "mm_get() failed! No node available. **mm_put() may segfault**. Returning NULL!");
		return NULL;
	}
	free->isfree = 0;			// mark chosen chunk as allocated (will cast to chunk * later)
	SPLICEOUT(MM,free);			// remove chunk from linked list	
	chunk * alloc = (chunk *) free;		// formally designate as allocated chunk
	if (alloc->SZ-size-sizeof(chunk) >= sizeof(freechunk))	// is there room for a free chunk at end?
	{
		int allocsize = size + sizeof(chunk);				// memory required for newly allocated chunk
		chunk * nextchunk = (chunk *)((void *)alloc + alloc->SZ);	// pointer to adjacent chunk (must edit its PREVSZ field)
		freechunk * newfree = (freechunk *)((void *)alloc + allocsize);	// location of new free chunk
		newfree->isfree = 1;
		newfree->SZ = alloc->SZ - allocsize;				// fill rest of space
		alloc->SZ = allocsize;						// finally set allocated chunk size
		newfree->PREVSZ = alloc->SZ;
		if (nextchunk != MM->front+MM->size)				// only edit if it exists
			nextchunk->PREVSZ = newfree->SZ;			// update linear neighbor
		SPLICEIN(MM,newfree);		// now splice new free chunk into 2-list (consolidation will never be necessary)
	}					// else new allocated chunk inherits entire size of previously free chunk (perhaps more than asked for)		
	return (void *)alloc + sizeof(chunk);	// return pointer to data section
}

void mm_put (mm_t *MM, void *chunk_arg) 
{
	freechunk * newfree = (freechunk *) ((void *)chunk_arg - sizeof(chunk));// cast to free
	newfree->isfree = 1;							// mark as free
	if ((void *)((void *)newfree+newfree->SZ) != MM->front+MM->size)	// this is not the last node, we can check the next one
	{
		freechunk * nextchunk = (freechunk *) ((void *)newfree + newfree->SZ);	// assume its a freechunk.  It cannot be ourself since SZ > 0
		if (nextchunk->isfree == 1)					// next adjacent chunk is a free chunk
		{
			SPLICEOUT(MM,nextchunk);				// remove next chunk from linked list
			CONSOLIDATE(MM,newfree,nextchunk);			// merge forward neighbor into this chunk
		}
	}
	freechunk * prevchunk = (freechunk *) ((void *)newfree - newfree->PREVSZ);
	if (newfree != prevchunk && prevchunk->isfree == 1)		// prev adjacent chunk is a free chunk and is not ourself
	{
		SPLICEOUT(MM,prevchunk);				// remove prev chunk from linked list
		CONSOLIDATE(MM,prevchunk,newfree);			// merge our chunk into previous free chunk
		SPLICEIN(MM,prevchunk);					// reinsert previous chunk into linked list		
	} else {
		SPLICEIN(MM,newfree);					// otherwise splice in our chunk
	}
}

void mm_release (mm_t *MM) 
{
	free(MM->front);
}
