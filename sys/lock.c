/* lock.c - lock, rampup_priority, maxprio_writer */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lock.c  --  Acquisition of a lock for read/write 
 *------------------------------------------------------------------------
 */
int lock(int ldes1, int type, int priority)
{
    STATWORD ps;
    disable(ps);
    struct	lentry	*lptr;
    int i;

    /* Check if it is a valid descriptor and the lock state should be used - not free */
    if(isbadlock(ldes1) || (lptr = &locktab[ldes1])->lstate == LFREE)
    {
        restore(ps);
        return(SYSERR);
    }

    /* if the lock has been created and no process is holding the lock*/
    if(lptr->ltype == NEITHER && lptr->lstate == LUSED)
    {
        //kprintf("\nEntering case1 - clause 1 lock.c\n");
        // Update the process lock parameters
        proctab[currpid].plockheld[ldes1] = 1;
        proctab[currpid].plwaitprio = priority;
        proctab[currpid].plockid = -1;

        //Update the lock parameters with the current process
        lptr->lprocess[currpid] = 1;
        lptr->ltype = type;
        lptr->lprio = priority;

        if(type == READ)
            lptr->lnum_read++; 

        restore(ps);
        return(OK);
    }

    /*Write lock is requested, or the lock type is Write */
    else if((type == READ && lptr->ltype == WRITE && lptr->lstate == LUSED) || type == WRITE)
    {
        //kprintf("\nEntering case2 - clause 2 lock.c ltype - %d,type - %d, priority-%d\n",lptr->ltype,type,priority);
        proctab[currpid].plockid = ldes1;
        proctab[currpid].ltype = type;
        proctab[currpid].pstate = PRWAIT;
        proctab[currpid].plstartwait = ctr1000;
        proctab[currpid].plockheld[ldes1] = 1;
        proctab[currpid].plwaitprio = priority;
        
        /* Insert into wait queue */
        //kprintf("Inserting to queue process %d  priority %d\n",currpid,priority);
        insert(currpid, locktab[ldes1].lqhead, priority);\

        for(i=0;i<NPROC;i++)
            if(proctab[currpid].pprio >= proctab[i].pprio && lptr->lprocess[i]) {
                rampup_priority(ldes1);
                break;
            }
        
        resched();
        
        locktab[ldes1].lprocess[currpid] = 1;
		if(type == READ)
			locktab[ldes1].lnum_read ++;

        proctab[currpid].plockid = -1;
        locktab[ldes1].ltype = type;

        
        restore(ps);
        return(OK);
    }
    
    /*Read lock is requested, or the lock type is READ */
    else if((lptr->ltype == READ && lptr->lstate == LUSED) || type == READ)
    {
        int highPrioWrite = maxprio_writer(ldes1);
        //kprintf("\nEntering case3 - HighPriowrite - %d  Priority - %d\n",highPrioWrite, priority);
        /* Writer with lock priority less than or equal to current lock priority*/
        if(highPrioWrite <= priority){
            //kprintf("\nEntering case4 - clause 4 lock.c\n");
            locktab[ldes1].lprocess[currpid] = 1;
            locktab[ldes1].lnum_read ++;
            proctab[currpid].plockheld[ldes1] = 1;

            restore(ps);
            return(OK);
        }
        else{
        
            proctab[currpid].plockid = ldes1;
            proctab[currpid].ltype = type;
            proctab[currpid].pstate = PRWAIT;
            proctab[currpid].plstartwait = ctr1000;
            proctab[currpid].plockheld[ldes1] = 1;
            proctab[currpid].plwaitprio = priority;
           
            /* Insert into wait queue */
            insert(currpid, locktab[ldes1].lqhead, priority);
           
            for(i=0;i<NPROC;i++)
                if(proctab[currpid].pprio >= proctab[i].pprio && lptr->lprocess[i]) {
                    rampup_priority(ldes1);
                    break;
                }
            
            resched();
            locktab[ldes1].lprocess[currpid] = 1;
            locktab[ldes1].lnum_read ++;
            proctab[currpid].plockid = -1;
            locktab[ldes1].ltype = type;
            
            restore(ps);
            return(OK);
        }
    }
}

/* Return the priority of the highest writer process*/
int maxprio_writer(int ldes){
    int highPrioWrite = -1;

    if(isempty(locktab[ldes].lqhead))
        return highPrioWrite;

    int procnxt = q[locktab[ldes].lqhead].qnext;
    while(procnxt != locktab[ldes].lqtail )   
    { 
        if((q[procnxt].qkey >= highPrioWrite) && proctab[procnxt].ltype == WRITE)
            highPrioWrite = q[procnxt].qkey;

        procnxt = q[procnxt].qnext;
    }
    return highPrioWrite;
}

/* Ramp up priority with priority less than max process */
int rampup_priority(int ldes){
    int i,maxprocess_prio = -1;
    //kprintf("rampup Priority begin\n");

    /* Identify the maximum process priority in wait queue*/
    if(isempty(locktab[ldes].lqhead))
        maxprocess_prio = -1;

    int procnxt = q[locktab[ldes].lqhead].qnext;
    //kprintf("procnxt - %d     proc priority-%d\n",procnxt,proctab[procnxt].pprio);
    while(procnxt != locktab[ldes].lqtail )   
    { 
        if((proctab[procnxt].pprio >= maxprocess_prio) ){
            maxprocess_prio = proctab[procnxt].pprio;
            //kprintf("maxprocess_prio is %d\n",maxprocess_prio);
        }

        procnxt = q[procnxt].qnext;
    }
    //kprintf("The max process priority - %d\n",maxprocess_prio);
    /* Update the priority only if the current lock desc is held */
    for(i=0; i<NPROC; i++)
    {   
        if(locktab[ldes].lprocess[i]){
            //kprintf("process %d     pinh - %d\n",i,proctab[i].pinh);
            if( proctab[i].pinh < maxprocess_prio){    
                proctab[i].pprio = maxprocess_prio;
                if (proctab[i].plockid != -1)
                    rampup_priority(proctab[i].plockid);

            }
            else{
                proctab[i].pprio = proctab[i].pinh;
                if(proctab[i].plockid != -1)
                    rampup_priority(proctab[i].plockid);
            }
        }   
    }
    //kprintf("rampup priority done\n");
}