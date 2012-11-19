/***********************
Author: Hemant Yadav
***********************/
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define INTERVAL 0
#define INTERVAL_USEC 800000
#define MEMSZ 64
#define HOW 8

/* 
   chunk, freechunk designed so that a freechunk can be casted
   to a chunk with all fields remaining accurate
*/

typedef struct chunk chunk;
typedef struct freechunk freechunk;

struct chunk		// all chunks have this form
{
	int isfree;		// 1 if chunk is a freechunk
	int PREVSZ;
	int SZ; 
};

struct freechunk	// free chunks have a little extra
{
	int isfree;		// should always be 1 for a valid freechunk
	int PREVSZ;
	int SZ;
	freechunk * FD;
	freechunk * BK;
};

typedef struct
{
	void* front;		// pointer to first usable space for nodes (may be a freechunk)
	freechunk* freelist;	// pointer to first free node in sorted doubly-linked free list
	int size;		// entire size of memory allocated to manager
} mm_t;


int  mm_init (mm_t *MM, int tsz);
void* mm_get (mm_t *MM, int neededSize);
void mm_put (mm_t *MM, void *chunk);
void  mm_release (mm_t *MM);
double comp_time (struct timeval times, struct timeval timee);
