#include "usertraps.h"
#include "misc.h"

#define PRO1 "1.dlx.obj"
#define PRO2 "2.dlx.obj"
#define PRO3 "3.dlx.obj"
#define PRO4 "4.dlx.obj"
#define PRO5 "5.dlx.obj"
#define PRO6 "6.dlx.obj"

void main (int argc, char *argv[])
{
  int num_hello_world = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
  sem_t s_procs_completed30;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed30_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }
  // Convert string from ascii command line argument to integer number
  //num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  //Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  if ((s_procs_completed30 = sem_create(-29)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(s_procs_completed30, s_procs_completed30_str);
  // Create Problem 1 processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): for the case Hello World\n", getpid());
  process_create(PRO1, s_procs_completed_str, NULL);

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }


  // Create Problem 2 processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): for the case Access memory beyond the maximum virtual address\n", getpid());
  process_create(PRO2, s_procs_completed_str, NULL);

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }

  // Create Problem 3 processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): for the case Access memory beyond the page\n", getpid());
  process_create(PRO3, s_procs_completed_str, NULL);

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }

  // Create Problem 4 processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): for the case Use more than one stack\n", getpid());
  process_create(PRO4, s_procs_completed_str, NULL);

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }


  // Create Hello World processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Creating %d hello world's in a row\n", getpid(), 100);
  for(i=0; i<100; i++) {
    Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
    process_create(PRO1, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
  }

  // Create Hello World processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Creating %d hello world's in a row, and make them run at the same time\n", getpid(), 30);
  for(i=0; i<30; i++) {
    //Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
    process_create(PRO6, s_procs_completed30_str, NULL);
  }
 if (sem_wait(s_procs_completed30) != SYNC_SUCCESS) {
   Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed30, argv[0]);
   Exit();
 }

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
