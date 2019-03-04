#include <xinu.h>

int lck;

#ifndef NLOCKS
#define NLOCKS 50
#endif
#ifndef READ 
#define READ 0
#define READL READ
#endif
#ifndef WRITE 
#define WRITE 1
#define WRITEL WRITE
#endif
#ifndef DELETED
#define DELETED (-4)
#endif

int count = 0;
int lcks [NLOCKS];

void reader1( int, int, int );
void writer1( int, int, int );
void loop(int);
void counter(int, int, int);
void read_count(int, int, int);
void holder10(int, int, int, int, int);
void waiter10(int, int , int);
void lp ( int lck, int num, int prio );

void test0(void);
void test1(void);
void test2(void);
void test3(void);
void test4(void);
void test5(void);
void test6(void);
void test7(void);
void test8(void);
void test9(void);
void test10(void);
void test11(void);
void test12(void);


int main(int argc, char** argv) {
/*	kprintf("\n\nCS503 Lab2 \n\r");
	kprintf("\n\nRunning test 0\n\r");
	test0();
	kprintf("\n\nRunning test 1\n\r");
	test1();
	kprintf("\n\nRunning test 2\n\r");
	test2();
	kprintf("\n\nRunning test 3\n\r");
	test3();
	kprintf("\n\nRunning test 4\n\r");
	test4();
	kprintf("\n\nRunning test 5\n\r");
	test5();
	kprintf("\n\nRunning test 6\n\r");
	test6(); 

	test7();
	test8();
	test9();
	test10(); */

	test5();

	test11();
	return 0;
}

void test10(void){
	//Create a process that holds multiple locks
	//Create multiple process that waits on each lock
	//Kill the first process while it holds the locks
	//Check for output of the other processes

	kprintf("TEST 10 \n\n");
	lcks[0] = lcreate();
	lcks[1] = lcreate();
	lcks[2] = lcreate();
	pid32 pid;
	pid = create(holder10, 2000, 30, "writer8", 5, lcks[0], lcks[1], lcks[2], 0,0);
	resume(pid);
	sleepms(500);
	resume(create(waiter10, 2000, 30, "reader8", 3, lcks[0], 0, 0));
	resume(create(waiter10, 2000, 30, "reader8", 3, lcks[1], 1, 0));
	resume(create(waiter10, 2000, 30, "reader8", 3, lcks[2], 2, 0));
	sleep(2);
	
	kprintf("Killing holder\n");
	kill(pid);

	sleep(2);
	
	kprintf("TEST 10 DONE\n\n");
}

void holder10(int ldes1, int ldes2, int ldes3, int num, int prio){
	kprintf("Holder\n");
	lock(ldes1, WRITE, prio);
	lock(ldes2, WRITE, prio);
	lock(ldes3, WRITE, prio);
	kprintf("Holder acquired locks... entering loop\n");
	while(1){
		sleep(1);
	}
}

void waiter10(int ldes1, int num, int prio){
	kprintf("waiter%d\n", num);
	int a;
	a = lock(ldes1, WRITE, prio);
	if(a != OK) kprintf("waiter error\n");
	kprintf("waiter%d has the lock!\n", num);
	a = releaseall(1, ldes1);
	if(a != OK) kprintf("waiter error2\n");
}

/* Tests writer priority in case of tie */
/* Should output writer1 lock, writer2 lock, reader1 lock */
void test8(void){
	
	kprintf("TEST 8\n\n");
	kprintf("Lock Order Should be: Writer0, Writer1, Reader0\n");
	lck = lcreate();
	if(lck == SYSERR) kprintf("Test 8 lcreate error");

	resume(create(writer1, 2000, 30, "writer8", 3, lck, 0,10));
	sleepms(100);
	resume(create(reader1, 2000, 30, "reader8", 3, lck, 0, 0));
	resume(create(writer1, 2000, 30, "writer8", 3, lck, 1, 0));
	sleep(10);
	kprintf("TEST 8 DONE\n\n");
}

void test9(void){
	
	kprintf("TEST 9\n\n");
	kprintf("Lock Order Should be: Writer0, Reader0, Writer1\n");
	lck = lcreate();
	if(lck == SYSERR) kprintf("Test 8 lcreate error");

	resume(create(writer1, 2000, 30, "writer8", 3, lck, 0,10));
	sleepms(100);
	resume(create(reader1, 2000, 30, "reader8", 3, lck, 0, 1));
	resume(create(writer1, 2000, 30, "writer8", 3, lck, 1, 0));
	sleep(10);
	kprintf("TEST 9 DONE\n\n");
}



void test7(void){

	int a;
	count = 0;
	kprintf("TEST 7\n\n");
	lck = lcreate();
	if(lck == SYSERR) kprintf("Test 7 lcreate error");
	resched_cntl(DEFER_START);
	resume(create(read_count, 2000, 30, "readcount0", 2, lck, 0, 0));
	resume(create(counter, 2000, 30, "counter0", 2, lck, 0,10));
	resume(create(counter, 2000, 30, "counter1", 2, lck, 1,10));
	resume(create(counter, 2000, 30, "counter2", 2, lck, 2,10));
	kprintf("all processes created \n");
	resched_cntl(DEFER_STOP);
	sleep(15);
	kprintf("Count = %d\n", count);
	kprintf("TEST 7 DONE\n\n");
}

void counter(int ldes, int num, int prio){
	kprintf("Counter %d started\n", num);
	int i;
	for(i = 0; i< 100000; i++){
		lock(ldes, WRITE, prio);
		count++;
		releaseall(1, ldes);
	}
	kprintf("%dW\n", num);
}

void read_count(int ldes, int num, int prio){
	while(1){
		lock(ldes, READ, prio);
		kprintf("%d %d\n", num, count);
		if(count >= 3*100000){
			releaseall(1, ldes);
			kprintf("%dR\n", num);
			return;
		}
		releaseall(1, ldes);
		sleepms(0);
	}
}


/*Test0	- Regression testing
 *Expected Output:
 *	TEST0 
 *	TEST0	DONE
 */

void test0()
{
	int i, j, a;

	for( j = 0; j < 4; j++ )	
	{
		for( i = 0; i < NLOCKS; i++ )
		{
			lcks[i] = lcreate();	
			if( lcks[i] == SYSERR )
			{
				kprintf(" TEST0: lcreate failed ..DONE\n\r");
				return;
			}
		}
		for( i = 0; i < NLOCKS; i++ )
		{
			a = lock( lcks[i], READ, 0 );
			if( a == SYSERR )
			{
				kprintf(" TEST0: lock failed ..DONE\n\r");
				return;
			}
		}
		for( i = 0; i < NLOCKS; i++ )
		{
			a = releaseall( 1,lcks[i] );
			if( a == SYSERR )
			{
				kprintf(" TEST0: release failed ..DONE\n\r");
				return;
			}
		}
		for( i = 0; i < NLOCKS; i++ )
		{
			a = ldelete( lcks[i] );
			if( a == SYSERR )
			{
				kprintf(" TEST0: release failed ..DONE\n\r");
				return;
			}
		}
	}
	kprintf(" TEST0	DONE \n\r");
}

/* Test 1 - Test for basic lcreate, lock, release ( read lock ) 
 *
 * Expected Output : 
 *	TEST1
 * 
 *
 *	Reader1: lock .. 
 *	Reader2: lock .. 
 *	Reader3: lock .. 
 *
 *	Reader1: Releasing .. 
 *	Reader2: Releasing .. 
 *	Reader3: Releasing .. 
 * 	( The order doesnt matter . The last 3  
 *	  statements will be printed after approx 3 seconds )
 *	TEST1: Completed
 *	       
 */

void test1()
{
	int  a;
	lck = lcreate();
	
	if( lck == SYSERR )
	{
	  kprintf(" TEST1: error in lcreate()..DONE\n\r");
	  return;
	}

	resume( create( reader1, 2000, 30, "reader", 3, lck, 1, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3, lck, 2, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3, lck, 3, 0 ));

	/* wait for them to complete and then delete the lock */
	sleep( 5 );
	kprintf(" TEST1: deleting lock\n");
	a = ldelete( lck );

	if( a != OK )
	{
		kprintf(" TEST1: error in ldelete()..DONE\n\r");
		return;
	}
	kprintf(" TEST1: DONE \n\r");
	

}

void reader1 ( int lck, int num, int prio )
{
	int a;
	a = lock( lck, READ, prio );
	if( a != OK )
	{
	  kprintf(" Reader%d: lock failed %d ..\n\r", num, a ); 
	  return;
	}

	kprintf(" Reader%d: Lock ..\n\r", num );
	sleep(3);
	kprintf(" Reader%d: Releasing ..\n\r", num );

	a = releaseall( 1,lck );
	if( a != OK )
		kprintf(" Reader%d: Lock release failed %d ..\n\r", num, a ); 
//	else
//		kprintf(" Reader%d: Lock release done ..\n\r", num ); 
}

/* Test2 - Test for basic lcreate, lock, release ( for a write lock ) 
 *
 * Expected output:
 *	Writer1: Lock ..
 *	Writer1: Releasing  ..
 *     	(after 3 seconds )
 *	Writer2: Lock ..
 *	Writer2: Releasing ..
 *     	(after 3 seconds )
 *	Writer3: Lock ..
 *	Writer3: Releasing ..
 *     The order is not important. What is important is that the lock/release 
 * for a process should not be interspersed with the lock/release for another process
 */


void test2()
{
	int a; 

	kprintf("	TEST2	\n\r\n\r");
	lck = lcreate();

	if( lck == SYSERR )
	{
	  kprintf(" TEST2: error in lcreate()..DONE\n\r");
	  return;
	}
	
	resume( create( writer1, 2000, 30, "writer", 3, lck, 1, 0 ));
	resume( create( writer1, 2000, 30, "writer", 3, lck, 2, 0 ));
	resume( create( writer1, 2000, 30, "writer", 3, lck, 3, 0 ));

	/* Wait for the writers to complete and then delete the lock */

	sleep(15);
	a = ldelete( lck );

	if( a != OK )
	  kprintf(" TEST2: error in ldelete()..\n\r");

	kprintf("	TEST2	DONE\n\r");
	
}

void writer1 ( int lck, int num, int prio )
{
	int a;
	kprintf(" Writer%d: Attempting lock\n", num);
	a = lock( lck, WRITE, prio );
	if( a != OK )
	  {
	    kprintf(" Writer%d: lock failed %d ..\n\r", num, a ); 
	    return;
	  }
	
	kprintf(" Writer%d: Lock ..\n\r", num );
	sleep(3);
	kprintf(" Writer%d: Releasing ..\n\r", num );

	a = releaseall(1, lck );
	if( a != OK )
	  kprintf(" Writer%d: Lock release failed %d ..\n\r", num,a ); 
	return;
}


/* Test3: Testing for SYSERRs in case of erroneous inputs 
 * Expected output - SYSERRs in all cases 
 */

void test3()
{
	/* Locking without creating */
	
	lck = 2;
	kprintf(" lock(2,READ) without lcreate() : %d \n\r", lock( lck, READ,0 ) ); 

	/* Locking with invalid id */

	lck = -2;
	kprintf(" lock(-2,READ)  : %d \n\r", lock( lck, READ,0 ) ); 


	/* Locking with an invalid mode */

	lck = lcreate();
	if( lck == SYSERR ) 
		kprintf(" TEST3: lcreate() failed .. DONE\n\r");

	kprintf(" lock(lck,INVALID) : %d \n\r", lock( lck, -9,0 ) ); 
	ldelete(lck);

	/* Deleting without creating  */

	lck = 10;
	kprintf(" ldelete(lck) without lcreate() : %d \n\r", ldelete( lck ) ); 


	/* Deleting with an invalid ID */

	lck = -1;
	kprintf(" ldelete(-1)  : %d \n\r", ldelete( lck ) ); 
}

/*Test4 - readers and writers of equal priority 
 *Expected Output: 
 *	Reader1: lock .. 
 *	Reader2: lock .. 
 *	Reader1: Releasing  .. 
 *	Reader2: Releasing  .. 
 *	Writer1: Lock ..
 *	Writer1: Releasing ..
 *	Writer2: Lock ..
 *	Writer2: Releasing ..
 *	Reader3: lock .. 
 *	Reader4: lock .. 
 *	Reader3: Releasing  .. 
 *	Reader4: Releasing  .. 
 * 	( previous 2 statements can be in any order )
 */

void test4()
{
	int a;
	
	kprintf("	TEST4	\n\r\n\r");
	
	lck = lcreate( );

	resume( create( reader1, 2000, 30, "reader1", 3, lck, 1, 0 ));
	resume( create( reader1, 2000, 30, "reader2", 3, lck, 2, 0 ));
	resume( create( writer1, 2000, 30, "writer1", 3, lck, 1, 0 ));

	sleep(11);
	resume( create( writer1, 2000, 30, "writer2", 3,lck, 2, 0 ));
	resume( create( reader1, 2000, 30, "reader3", 3,lck, 3, 0 ));
	resume( create( reader1, 2000, 30, "reader4", 3,lck, 4, 0 ));

	sleep(11);
	
	a = ldelete(lck);

	if( a != OK )
		kprintf(" TEST4: error in ldelete()..\n\r");

	kprintf("	TEST4 DONE	\n\r\n\r");
}

/* Test5
 * Testing priority inheritance on a single lock acquired by multiple processes  
 * Expected output: 
 * Reader1: Lock ..
 * Reader2: Lock..
 * Reader3: Lock..
 * pid of medium priority process = x
 * Reader1: Releasing
 * Reader2: Releasing
 * Reader3: Releasing
 * Writer1: Lock
 * Writer 1: Releasing
 * pid x exiting from loop
 * (The order of the readers acquiring / releasing the locks does not matter )
*/
void test5() {
  int lk = lcreate();
  int mpid;

  resume( create( reader1, 2000, 30, "reader", 3, lk, 1, 0 ));
  resume( create( reader1, 2000, 30, "reader", 3, lk, 2, 1 ));
  resume( create( reader1, 2000, 30, "reader", 3, lk, 3, 2 ));
  /* The 3 readers have acquired the lock */
  chprio(getpid(),60);
  resume(mpid = create(loop,2000,40,"loop",1,10));
  kprintf("pid of medium priority process = %d\n\r",mpid);
  resume(create(writer1,2000,50,"writer",3,lk,1,0));
  chprio(getpid(),20);
  sleep(10);
  if(ldelete(lk)!=OK) 
    kprintf("Test5 : Error while deleting lock\n\r");
  else
	  kprintf("deleted!!!\r\n");
  kprintf("Test 5 done..\n\r");
}

/* Test that priorities are set and released under PRI INH properly */
void test11(){
	kprintf("===== TEST 11 =====\n");
	int lk = lcreate();
	int lpid, mpid, hpid;
	
	resume(lpid = create(lp, 2000, 30, "lprio", 3, lk, 1, 0));
	sleepms(250);
	chprio(getpid(), 60);
	resume(mpid = create(loop, 2000, 40, "loop",1,10));
	sleepms(250);
	resume(hpid = create(writer1, 2000, 50, "hprio", 3, lk, 1, 0));
	
	sleep(10);
	kprintf("Priorities: lpid = %d, mpid = %d, hpid = %d\n", proctab[lpid].prinh, proctab[mpid].prinh, proctab[hpid].prinh);
	kprintf("TEST 11 FINISHED\n");
}

void lp ( int lck, int num, int prio )
{
	int a;
	a = lock( lck, READ, prio );
	if( a != OK )
	{
	  kprintf(" LP%d: lock failed %d ..\n\r", num, a ); 
	  return;
	}

	kprintf(" LP%d: Lock ..\n\r", num );
	sleep(3);
	kprintf("In low prio process: prio = %d\n", proctab[getpid()].prinh);
	kprintf(" LP%d: Releasing ..\n\r", num );

	a = releaseall( 1,lck );
	if( a != OK )
		kprintf(" LP%d: Lock release failed %d ..\n\r", num, a ); 
//	else
//		kprintf(" Reader%d: Lock release done ..\n\r", num ); 
}

/*Test6 - DELETED test
*Points - 10
*Expected Output:
*      Reader1: Lock ..
*      Writer1: Lock failed (value of DELETED here)..
*      Writer2: Lock failed (value of DELETED here)..  (order of these 3 doesn't matter)
*      Reader2: Lock failed SYSERR..
*      Reader1: Releasing ..
*      Reader1: Lock Release failed SYSERR..
*      TEST7:  DONE
*
*/

void test6()
{
       int a;

       kprintf("       TEST6   \n\r\n\r");

       lck = lcreate();

       if( lck == SYSERR )
       {
               kprintf(" TEST7: lcreate failed ...DONE\n\r\n\r");
               return;
       }

       resume( create( reader1, 2000, 30, "reader", 3, lck, 1, 0 ));
       resume( create( writer1, 2000, 30, "writer", 3, lck, 1, 0 ));
       resume( create( writer1, 2000, 30, "writer", 3, lck, 2, 0 ));

       a = ldelete(lck);
       if( a != OK )
       {
               kprintf(" TEST6: ldelete failed ...\n\r\n\rDONE");
               return;
       }
       resume( create( reader1, 2000, 30, "reader", 3, lck, 2, 0 ));

       sleep(5);
       kprintf(" TEST6: DONE \n\r" );

}


void loop(int timetoloop) {
  kprintf("pid %d starting loop\n\r", getpid());
  const unsigned int start_time = clktime;
  while((clktime - start_time) <= timetoloop);
  kprintf("pid %d exiting from loop \n\r",getpid());
}

  
