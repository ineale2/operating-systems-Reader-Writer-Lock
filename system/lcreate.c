/*	lcreate.c - lcreate	*/
#include <xinu.h>

local int32 newlock(void);

int32 lcreate() {

	intmask mask;
	int32 lock;

	mask = disable();
	lock = newlock();
	restore(mask);
	//NOTE: This handles the case where newlock() returns SYSERR
	return lock;
}

int32 newlock(void){ /*Assumes interrupts are disabled */
	static int32 nextlock = 0;
	int32 lock;
	int32 i;

	for(i = 0; i< NLOCKS; i++){
		lock = nextlock++;
		if(nextlock >= NLOCKS){
			nextlock = 0;
		}
		if(locktab[lock].lstate == L_FREE){
			//Found a free lock
			locktab[lock].lstate = L_USED;
			return lock;
		}
	}
	//For loop failed to find free spot
	return SYSERR;

}
