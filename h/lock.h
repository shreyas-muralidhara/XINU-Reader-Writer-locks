/* lock.h */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS
#define	NLOCKS		50	/* number of locks allowed, if not defined	*/
#endif

#define NEITHER     0   /* Assigned lock type is Deleted */
#define READ        1   /* Assigned lock type is Read */
#define WRITE       2   /* Assigned lock type is Write */

#define LFREE       3   /* This lock is free */
#define LUSED       4   /* This lock is used */

/*typedef struct lproclist{
    int pid
}*/

struct lentry{          /* lock table entry */
    int lstate ;       /* the state LFREE or LUSED */
    int ltype;          /* Type of lock READ, WRITE, NEITHER */
    int lqhead;         /* queue index of head of list */
    int lqtail;         /* queue index of tail of list */
    
    int lprio;          /* Highest priority of the process waiting in queue */
    int lnum_read;      /* count of the  readers waiting for this lock */
    int lprocess[NPROC]; /* Process list waiting on the lock(max - NPROC) */  
};
extern struct lentry locktab[];  /* declare a lock entry type obejct */
extern int nextlock;
extern int numlock;
extern unsigned long ctr1000;   /* Clock time to measure the wait in Lock's queue */

#define isbadlock(l) (l<0 || l>=NSEM)

extern int maxprio_writer(int ldes);
extern int rampup_priority(int ldes);

#endif