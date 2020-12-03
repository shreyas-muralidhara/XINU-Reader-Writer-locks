/* releaseall.c - releaseall, assignNext_lqProcess */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * releaseall  --  Simultaneous release of multiple locks 
 *------------------------------------------------------------------------
 */

int releaseall(int numlocks, int ldes, ...){
    STATWORD ps;
    disable(ps);

    int i, descriptor;
    int flag = 0; 
    for(i=0;i<numlock;i++){
        descriptor = *(&ldes + i);

        if(isbadlock(descriptor) || locktab[descriptor].lstate == LFREE || proctab[currpid].plockheld[descriptor] == 0)
        {
            flag = 1;
            continue;
        }
        else if(locktab[descriptor].ltype == READ)
        {
            locktab[descriptor].lprocess[currpid] = 0;
            proctab[currpid].plockheld[descriptor] = 0;
            proctab[currpid].ltype = NEITHER;
            locktab[descriptor].lnum_read --;
            
            if(locktab[descriptor].lnum_read==0)
                assignNext_lqProcess(descriptor);
                    
        }

        else if(locktab[descriptor].ltype == WRITE)
        {
            locktab[descriptor].lprocess[currpid] = 0;
            proctab[currpid].plockheld[descriptor] = 0;
            proctab[currpid].ltype = NEITHER;

            assignNext_lqProcess(descriptor);
                
        }
    }
    if(flag == 0)
        return OK;
    else
        return SYSERR;
}


/*------------------------------------------------------------------------------------
 * assignNext_lqProcess  --  Release the locks from the lock queue and Rampup priority
 *------------------------------------------------------------------------------------
 */

void assignNext_lqProcess(int ldes)
{
    /* If queue is empty release the lock type */
    if(q[locktab[ldes].lqhead].qnext == locktab[ldes].lqtail){
        locktab[ldes].ltype = NEITHER;
        return ; 
    }

    /* Check if the process with highest priority in queue is WRITE type:
        If yes, then i. remove from queue, ii. rampup priority iii. reschedule*/
    int temp;
    int currproc = q[locktab[ldes].lqtail].qprev;
    int prevproc = q[currproc].qprev;
    int maxwrite = maxprio_writer(ldes);

    if(proctab[currproc].ltype == WRITE){
        dequeue(currproc);
        rampup_priority(ldes);
        ready(currproc, RESCHYES);
        return ;
    }


    while(prevproc != locktab[ldes].lqhead){

        /* check if the current process and previous priority are same and the prev process is WRITE type
          if yes compare the wait time, if the prev priority is greater then 
          i. remove from queue, ii. rampup priority iii. reschedule  */

        if(q[currproc].qkey == q[prevproc].qkey && proctab[prevproc].ltype == WRITE){
            if(proctab[prevproc].plstartwait - proctab[currproc].plstartwait <1000){
                dequeue(prevproc);
                rampup_priority(ldes);
                ready(prevproc, RESCHYES);
                return ;
            }
        }

        /* If the previous process is also READ Type and it is of different priority from the current process,
           then identify the max WRITE process from queue and release all the locks with priority greater than that process*/

        if(q[prevproc].qkey >= maxwrite && proctab[prevproc].ltype == READ)
        {
            temp = prevproc;
            prevproc = q[prevproc].qprev;
            dequeue(temp);
            ready(temp, RESCHNO);
            continue;
        }       

        prevproc = q[prevproc].qprev;
    }

    /* COntrol reaches here if the highest priority process is READ type
        i. remove from queue, ii. rampup priority iii. reschedule  */
    dequeue(currproc);
    rampup_priority(ldes);
    ready(currproc, RESCHYES);
    return ;  
}