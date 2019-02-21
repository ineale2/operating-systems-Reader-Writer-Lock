/*	linit.c	- linit	initialize lock system */
#include <xinu.h>

struct lockent locktab[NLOCKS];

void linit(void) {
  
	/* Walk through lock table and initialize every lock entry */ 
	struct lockent *lptr;
	int32 i;

	for(i = 0; i < NLOCKS; i++){
		lptr = &locktab[i];
		lptr->lstate 	 = L_FREE;
		lptr->ltype		 = FREE;
		lptr->numReaders = 0;
		lptr->lqueue 	 = newqueue();
	}
}
