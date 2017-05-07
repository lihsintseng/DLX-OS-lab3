#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int child_pid;
  int current_id;
  int temp;
  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 
  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  
  // Now print a message to show that everything worked
  //Printf("The main program process ID is %d\n", (int) getpid());

  child_pid = fork();


  printpage(); 
  
  //Printf("fork done \n");
  if(child_pid != 0)
    Printf("Parent: fork (%d): Done!\n", getpid());
  else
    Printf("Child: fork (%d): Done!\n", getpid());


  // Signal the semaphore to tell the original process that we're done
//  if(child_pid != 0){
    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
      //Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
      Exit();
    }
//  }


}
