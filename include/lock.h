/*  lock.h	*/

#define FREE  0
#define READ  1
#define WRITE 2

#define DELETED -2

#define L_USED 1
#define L_FREE 0

#define HELD 1
#define NOT_HELD 0  

struct	lockent {
	byte lstate; 			/* Either entry in locktab is L_USED or L_FREE 			*/
	qid16 lqueue;			/* Queue associated with this lock 		 				*/
	uint8  ltype;			/* Whether the lock is READ, WRITE, or FREE 		    */
	uint16 numReaders;		/* Number of readers that hold this lock 				*/
	pri16 maxprio;			/* Maximum priority of processes in the wait queue  	*/  	
};

/* Lab 2 lock table */

extern struct lockent locktab[NLOCKS];


#define	isbadlock(ldes)	((int32)(ldes) < 0 || (ldes) >= NLOCKS)
