/* linit.c - linit */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * linit  --  initialize the lock 
 *------------------------------------------------------------------------
 */

int nextlock;
int numlock;

void linit(){
    int i,j;
    struct lentry *lptr;
    numlock = 0;

    for (i=0 ; i<NLOCKS ; i++) 	/* initialize locks */
    {
		(lptr = &locktab[i])->lstate = LFREE;
        lptr->lnum_read = 0;
        lptr->lqhead = newqueue();
		lptr->lqtail = 1 + (lptr->lqhead);
        lptr->lprio = -1;
        lptr->ltype = NEITHER;
        
        for(j=0; j<NPROC; j++)
            lptr->lprocess[i] = 0;
	}
    kprintf("Initialization complete\n");
}