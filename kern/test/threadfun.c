#include <types.h>
#include <lib.h>
#include <thread.h>
#include <synch.h>
#include <test.h>

#define NTHREADS  8
#define INCREMENT 10000

static struct semaphore* tsem = NULL;
static struct lock* thread_lock = NULL;
static struct spinlock thread_spinlock;

static unsigned long counter = 0;


static void init_sem(void)
{
  if (tsem==NULL) {
    tsem = sem_create("tsem", 0);
    
    if (tsem == NULL) {
      panic("threadtest: sem_create failed\n");
    }
  }
}

static void destroy_sem(void)
{
  if (tsem != NULL) {
    sem_destroy(tsem);
    tsem = NULL;
  }
}

static void init_lock(void)
{
  if (thread_lock == NULL) {
    thread_lock = lock_create("thread_lock");
    
    if (thread_lock == NULL) {
      panic("threadtest: lock_create failed\n");
    }
  }
}

static void destroy_lock(void) {
  if (thread_lock != NULL) {
    lock_destroy(thread_lock);
    thread_lock = NULL;
  }
}

static void mythreadset(void *junk, unsigned long num) 
{
  
  (void)junk;

  for (unsigned long i = 0; i < num; i++){

    counter++;
  }

  V(tsem);
}

static void runthreads(int num_threads, int num_inc)
{
  char name[16];
  int i, result;

  counter = 0;

  for (i=0; i< num_threads; i++) {
    snprintf(name, sizeof(name), "threadtest%d", i);
    result = thread_fork(name, NULL,
			 mythreadset,
			 NULL, num_inc);
    if (result) {
      panic("threadtest: thread_fork failed %s)\n", 
	    strerror(result));
    }
  }

  for (i=0; i< num_threads; i++) {
    P(tsem);
  }
}

/*int mythreadtest(int nargs, char *args[])
{
  (void)nargs;
  //(void)args;
  int input = *args[1] - 48;

  init_sem();
  kprintf("Starting my thread test ... \n");
  runthreads(input);
  kprintf("\nThread test done. \n");

  return 0;
  }*/

int unsafethreadcounter(int nargs, char *args[])
{

  unsigned long num_threads = NTHREADS;
  unsigned long num_inc = INCREMENT;

  if (nargs > 1)
    num_threads = atoi(args[1]);
  if (nargs > 2)
    num_inc = atoi(args[2]);

  init_sem();
  kprintf("Starting my thread test with %lu threads... \n", num_threads);
  runthreads(num_threads, num_inc);
  kprintf("\nThread test done. \n");
  kprintf("Counter is %lu -which should be %lu-\n", counter, num_threads * num_inc);
  destroy_sem();
  return 0;
}

static void mylockset(void *junk, unsigned long num)
{

  (void)junk;

  for (unsigned long i = 0; i < num; i++) {
    lock_acquire(thread_lock);
    counter++;
    lock_release(thread_lock);
  }

  V(tsem);
}

static void lockrunthreads(int num_threads, int num_inc)
{
  char name[16];
  int i, result;

  counter = 0;

  for (i=0; i< num_threads; i++) {
    snprintf(name, sizeof(name), "threadtest%d", i);
    result = thread_fork(name, NULL,
			 mylockset,
			 NULL, num_inc);
    if (result) {
      panic("threadtest: thread_fork failed %s)\n", 
	    strerror(result));
    }
  }

  for (i=0; i< num_threads; i++) {
    P(tsem);
  } 
}

int lockthreadcounter(int nargs, char* args[])
{

  unsigned long num_threads = NTHREADS;
  unsigned long num_inc = INCREMENT;

  if (nargs > 1)
    num_threads = atoi(args[1]);
  if (nargs > 2)
    num_inc = atoi(args[2]);

  init_sem();
  init_lock();
  kprintf("Starting my lockthread test with %lu threads... \n", num_threads);
  lockrunthreads(num_threads, num_inc);
  kprintf("\nThread test done. \n");
  kprintf("Counter is %lu -which should be %lu-...did it work?\n", counter, num_threads * num_inc);
  destroy_lock();
  destroy_sem();
  
  return 0;
}


static void myspinlockset(void *junk, unsigned long num)
{

  (void)junk;

  for (unsigned long i = 0; i < num; i++) {
    spinlock_acquire(&thread_spinlock);
    counter++;
    spinlock_release(&thread_spinlock);
  }

  V(tsem);
}

static void spinlockrunthreads(int num_threads, int num_inc)
{
  char name[16];
  int i, result;

  counter = 0;

  for (i=0; i< num_threads; i++) {
    snprintf(name, sizeof(name), "threadtest%d", i);
    result = thread_fork(name, NULL,
			 myspinlockset,
			 NULL, num_inc);
    if (result) {
      panic("threadtest: thread_fork failed %s)\n", 
	    strerror(result));
    }
  }

  for (i=0; i< num_threads; i++) {
    P(tsem);
  } 
}

int spinlockthreadcounter (int nargs, char* args[]) {

  unsigned long num_threads = NTHREADS;
  unsigned long num_inc = INCREMENT;

  if (nargs > 1)
    num_threads = atoi(args[1]);
  if (nargs > 2)
    num_inc = atoi(args[2]);

  init_sem();
  spinlock_init(&thread_spinlock);
  kprintf("Starting my spinlock thread test with %lu threads... \n", num_threads);
  spinlockrunthreads(num_threads, num_inc);
  kprintf("\nThread test done. \n");
  kprintf("Counter is %lu -which should be %lu-...did this work?\n", counter, num_threads * num_inc);
  spinlock_cleanup(&thread_spinlock);
  destroy_sem();
  
  return 0;

}
