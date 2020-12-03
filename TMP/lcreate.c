/* lcreate.c - lcreate, newlock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * lcreate  --  create and initialize a lock, returning its id
 *------------------------------------------------------------------------
 */
int lcreate()
{
    STATWORD ps;
    int lock;

    disable(ps);
    if((lock=newlock())==SYSERR) {
        restore(ps);
        return(SYSERR);
    }
    /* lqhead and lqtail were initialized at system startup */
    //kprintf("The lockdesc created is - %d\n\n", lock);
    restore(ps);
    return(lock);
}

/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
    int lock;
    int i;

    for(i=0;i<NLOCKS;i++){
        lock=nextlock--;
        if(nextlock < 0){
            nextlock = NLOCKS - 1;
            numlock = numlock + 1;
        }
        if(locktab[lock].lstate==LFREE) {
            locktab[lock].lstate = LUSED;
            return(lock);
        }
    }
    return(SYSERR);
}
     