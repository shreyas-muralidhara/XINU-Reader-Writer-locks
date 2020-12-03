/* task1.c - lock_test, lock_reader, lock_writer 
              sem_test, sem_reader, sem_writer  */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }


void sem_reader(int sem)
{
	kprintf("-start reader %s priority: %d to acquire Semaphore",proctab[currpid].pname,getprio(currpid));
	wait(sem);
	kprintf("\nProcess %s: acquired semaphore, sleep for 1 sec",proctab[currpid].pname,getprio(currpid));
	sleep(1);
        kprintf("\nPriority of %s: %d(no ramp up)",proctab[currpid].pname,getprio(currpid));
        kprintf("\n%s: release semaphore",proctab[currpid].pname);
	signal(sem);
}
void sem_writer(int sem)
{
	kprintf("-start writer %s priority: %d to acquire Semaphore",proctab[currpid].pname,getprio(currpid));
	wait(sem);
	kprintf("\nProcess %s: acquired semaphore, sleep for 1 sec",proctab[currpid].pname,getprio(currpid));
	sleep(1);
        kprintf("\nPriority of %s: %d(no ramp up)",proctab[currpid].pname,getprio(currpid));
        kprintf("\n%s: release semaphore",proctab[currpid].pname);
	signal(sem);
}

void semtest()
{
        int sem,p1,p2,p3;
	kprintf("\nSemaphore Test: implemntation of semaphore for basic priority inheritence\n");
        sem  = screate (1);
        assert (sem != SYSERR, "Test for Semaphore implemnetation with priority inhertence failed");

	p1=create(sem_reader,2000,10,"A",1,sem);
	p2=create(sem_reader,2000,20,"B",1,sem);
	p3=create(sem_writer,2000,30,"C",1,sem);
	
	kprintf("\nStarting A(reader process):\n");
	resume(p1);
	sleep(1);
	kprintf("\n\nStarting B(reader process):\n");
        resume(p2);
        sleep(1);
	kprintf("\n\nStarting C(writer process):\n");
        resume(p3);
        sleep(5);

        sdelete (sem);

	kill(p1);
	kill(p3);
	kill(p2);
	//kprintf("output=%s\n", output);

	kprintf("\n\nImplementation using Semaphore completed \n");
}


void lock_reader(int lck)
{
	kprintf("-start reader %s priority: %d to acquire lock",proctab[currpid].pname,getprio(currpid));
        lock(lck,READ, DEFAULT_LOCK_PRIO);
        kprintf("\nProcess %s acquired lock, sleep for 5 sec",proctab[currpid].pname,getprio(currpid));
        sleep(5);
	kprintf("\nPriority of %s: %d(ramped up)",proctab[currpid].pname,getprio(currpid));
	kprintf("\n%s releasing lock",proctab[currpid].pname);
        releaseall(1,lck);
}
void lock_writer(int lck)
{
	kprintf("-start writer %s priority: %d to acquire lock",proctab[currpid].pname,getprio(currpid));
        lock(lck,WRITE, DEFAULT_LOCK_PRIO);
	kprintf("\nProcess %s acquired lock, sleep for 5 sec",proctab[currpid].pname,getprio(currpid));
        sleep(5);
        kprintf("\nProcess %s releasing lock",proctab[currpid].pname);
        releaseall(1,lck);
}

void locktest()
{
	
	int lck,rd1,rd2,wr1;
	kprintf("\nLock Test: implemntation of lock for basic priority inheritence\n");
        lck  = lcreate ();
        assert (lck != SYSERR, "Test for lock implemnetation with priority inhertence failed");
	
	rd1=create(lock_reader,2000,10,"A",1,lck);
	rd2=create(lock_reader,2000,20,"B",1,lck);
	wr1=create(lock_writer,2000,30,"C",1,lck);	
	
	kprintf("\nStarting A(reader process):\n");
	resume(rd1);
	sleep(1);
	kprintf("\n\nStarting B(reader process):\n");
	resume(rd2);
	sleep(1);
	kprintf("\n\nStarting C(writer process):\n");
	resume(wr1);
	sleep(5);

        ldelete (lck);

	kill(rd1);
	kill(wr1);
	kill(rd2);

	kprintf("\n\nImplementation using Locks completed \n");
}