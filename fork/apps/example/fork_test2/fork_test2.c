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
  temp = 0;
  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  
  // Now print a message to show that everything worked
  Printf("Single: The main program process ID is %d\n", (int) getpid());
  Printf("Single: Pid %d: A value before fork is %d\n", (int)(getpid()), temp);

  child_pid = fork();

  if(child_pid != 0){
    Printf("Parent: This is the parent process, with id %d\n", (int) getpid());
    Printf("Parent: The child's process ID is %d\n", child_pid);
    Printf("Parent: Triggle Read only page in parent\n");
    temp = 1;
    Printf("Parent: After fork!!!  Current ID is %d, and the value is %d\n", getpid(), temp);
  }
  else{
    Printf("Child: This is the child process, with id %d\n", (int) getpid());
    Printf("Child: After fork!!!  Current ID is %d, and the value is %d\n", getpid(), temp);
  }
  Printf("\n");
//  Printf("The semaphore is %d \n", s_procs_completed);

  printpage(); 
  Printf("\n");
  
  if(child_pid != 0)
    Printf("Parent: fork (%d): Done!\n", getpid());
  else
    Printf("Child: fork (%d): Done!\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  //if(child_pid != 0){
    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
      Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
      Exit();
    }
  //}

}
