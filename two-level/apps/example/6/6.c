#include "usertraps.h"
#include "misc.h"
void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int x = 0;
  long long int i;
  int sum = 0;
  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  Printf("Hello World, %d\n", getpid());

  for(i = 0; i < 1000; i++){
    Printf("");
  }

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  
  Printf("Exit %d\n", getpid());
  // Now print a message to show that everything worked
 //  Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

}


