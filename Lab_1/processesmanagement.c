
/*****************************************************************************\
* Laboratory Exercises COMP 3500                                              *
* Author: Saad Biaz                                                           *
* Updated 5/16/2019 to distribute to students to do Lab 1                     *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/

typedef enum {TAT,RT,CBT,THGT,WT} Metric;


/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 10 
#define FCFS            1 
#define SJF             2
#define RR              3 


#define MAXMETRICS      5 



/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/




/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/

Quantity NumberofJobs[MAXMETRICS]; // Number of Jobs for which metric was collected
Average  SumMetrics[MAXMETRICS]; // Sum for each Metrics

/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void                 ManageProcesses(void);
void                 NewJobIn(ProcessControlBlock whichProcess);
void                 BookKeeping(void);
Flag                 ManagementInitialization(void);
void                 LongtermScheduler(void);
void                 IO();
void                 CPUScheduler(Identifier whichPolicy);
ProcessControlBlock *SJF_Scheduler();
ProcessControlBlock *FCFS_Scheduler();
ProcessControlBlock *RR_Scheduler();
void                 Dispatcher();

/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment operating systems. The parent    *
*           process is the "Operating Systems" managing the processes using   *
*           the resources (CPU and Memory) of the system                      *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {
   if (Initialization(argc,argv)){
     ManageProcesses();
   }
} /* end of main function */

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: Monitor Sources and process events (written by students)    *
\***********************************************************************/

void ManageProcesses(void){
  ManagementInitialization();
  while (1) {
    IO();
    CPUScheduler(PolicyNumber);
    Dispatcher();
  }
}

/* XXXXXXXXX Do Not Change IO() Routine XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
/***********************************************************************\
* Input : none                                                          *          
* Output: None                                                          *        
* Function:                                                             *
*    1) if CPU Burst done, then move process on CPU to Waiting Queue    *
*         otherwise (for RR) return Process to Ready Queue              *                           
*    2) scan Waiting Queue to find processes with complete I/O          *
*           and move them to Ready Queue                                *         
\***********************************************************************/
void IO() {
  ProcessControlBlock *currentProcess = DequeueProcess(RUNNINGQUEUE); 
  if (currentProcess){
    if (currentProcess->RemainingCpuBurstTime <= 0) { // Finished current CPU Burst
      currentProcess->TimeEnterWaiting = Now(); // Record when entered the waiting queue
      EnqueueProcess(WAITINGQUEUE, currentProcess); // Move to Waiting Queue
      currentProcess->TimeIOBurstDone = Now() + currentProcess->IOBurstTime; // Record when IO completes
      currentProcess->state = WAITING;
    } else { // Must return to Ready Queue                
      currentProcess->JobStartTime = Now();                                               
      EnqueueProcess(READYQUEUE, currentProcess); // Mobe back to Ready Queue
      currentProcess->state = READY; // Update PCB state 
    }
  }

  /* Scan Waiting Queue to find processes that got IOs  complete*/
  ProcessControlBlock *ProcessToMove;
  /* Scan Waiting List to find processes that got complete IOs */
  ProcessToMove = DequeueProcess(WAITINGQUEUE);
  if (ProcessToMove){
    Identifier IDFirstProcess =ProcessToMove->ProcessID;
    EnqueueProcess(WAITINGQUEUE,ProcessToMove);
    ProcessToMove = DequeueProcess(WAITINGQUEUE);
    while (ProcessToMove){
      if (Now()>=ProcessToMove->TimeIOBurstDone){
	ProcessToMove->RemainingCpuBurstTime = ProcessToMove->CpuBurstTime;
	ProcessToMove->JobStartTime = Now();
	EnqueueProcess(READYQUEUE,ProcessToMove);
      } else {
	EnqueueProcess(WAITINGQUEUE,ProcessToMove);
      }
      if (ProcessToMove->ProcessID == IDFirstProcess){
	break;
      }
      ProcessToMove =DequeueProcess(WAITINGQUEUE);
    } // while (ProcessToMove)
  } // if (ProcessToMove)
}

/***********************************************************************\    
 * Input : whichPolicy (1:FCFS, 2: SJF, and 3:RR)                      *        
 * Output: None                                                         * 
 * Function: Selects Process from Ready Queue and Puts it on Running Q. *
\***********************************************************************/
void CPUScheduler(Identifier whichPolicy) {
  ProcessControlBlock *selectedProcess;
  switch(whichPolicy){
    case FCFS : selectedProcess = FCFS_Scheduler();
      break;
    case SJF : selectedProcess = SJF_Scheduler();
      break;
    case RR   : selectedProcess = RR_Scheduler();
  }
  if (selectedProcess) {
    selectedProcess->state = RUNNING; // Process state becomes Running                                     
    EnqueueProcess(RUNNINGQUEUE, selectedProcess); // Put process in Running Queue                         
  }
}

/***********************************************************************\
 * Input : None                                                         *
 * Output: Pointer to the process based on First Come First Serve (FCFS)*
 * Function: Returns process control block based on FCFS                *
\***********************************************************************/
ProcessControlBlock *FCFS_Scheduler() {
  /* Select Process based on FCFS */
  ProcessControlBlock *selectedProcess = (ProcessControlBlock *) DequeueProcess(READYQUEUE);
  return(selectedProcess);
}



/***********************************************************************\                         
 * Input : None                                                         *                                     
 * Output: Pointer to the process with shortest remaining time (SJF)   *                                     
 * Function: Returns process control block with SJF                    *                                     
\***********************************************************************/
ProcessControlBlock *SJF_Scheduler() {
  /* Select Process with Shortest Remaining Time*/

  // Get the first process out of the ready queue to begin comparing job durations
  ProcessControlBlock *shortestProcess = (ProcessControlBlock *) DequeueProcess(READYQUEUE);
  
  // Loop through the entire ready queue to find the process with the shortest TotalJobDuration
  do {
    // Get the second process out of the ready queue and point to it with currentProcess
    ProcessControlBlock *currentProcess = DequeueProcess(READYQUEUE);

    if (currentProcess) {
      // If the current process' job duration is shorter than the current shortest job, update the shortestJobTime variable
      if (currentProcess->TotalJobDuration < shortestProcess->TotalJobDuration) {
        // Put the process with the larger job duration back in the queue
        EnqueueProcess(READYQUEUE, shortestProcess);
        // Update shortestProcess pointer to the process with the shorter job duration
        shortestProcess = currentProcess;
      }
      else {
        // Put the current process back on the ready queue
        EnqueueProcess(READYQUEUE, currentProcess);
      }
    }

  } while (currentProcess);
  
  return(shortestProcess);
}


/***********************************************************************\                                               
 * Input : None                                                         *                                               
 * Output: Pointer to the process based on Round Robin (RR)             *                                               
 * Function: Returns process control block based on RR                  *
 \***********************************************************************/
ProcessControlBlock *RR_Scheduler() {
  /* Select Process based on RR*/

  // Get process to run next
  ProcessControlBlock *selectedProcess = (ProcessControlBlock *) DequeueProcess(READYQUEUE);

  // Get Process that just ran
  ProcessControlBlock *runningProcess = (ProcessControlBlock *) DequeueProcess(RUNNINGQUEUE);

  if (runningProcess) {
    // Put process that ran back in ready queue
    EnqueueProcess(READYQUEUE, runningProcess);
    // Remove process that ran from the running queue
    DequeueProcess(RUNNINGQUEUE);
    // Put next process to run in the running queue
    EnqueueProcess(RUNNINGQUEUE, selectedProcess);
  }

  return(selectedProcess);
}

/***********************************************************************\  
 * Input : None                                                         *   
 * Output: None                                                         *   
 * Function:                                                            *
 *  1)If process in Running Queue needs computation, put it on CPU      *
 *              else move process from running queue to Exit Queue      *     
\***********************************************************************/
void Dispatcher() {
  double start;
  ProcessControlBlock *selectedProcess = DequeueProcess(RUNNINGQUEUE);

  if (selectedProcess) {

    // if process has not been on CPU yet.
    if (selectedProcess->TimeInCpu == 0) {
      selectedProcess->StartCpuTime = Now();
      NumberofJobs[CBT]++;
      NumberofJobs[RT]++;
      SumMetrics[RT] += (Now() - selectedProcess->JobArrivalTime);
    }

    // if job is done.
    if (selectedProcess->TimeInCpu >= selectedProcess->TotalJobDuration) {
      selectedProcess->JobExitTime = Now();
      selectedProcess->state = DONE;

      // update metrics
      SumMetrics[TAT] += (selectedProcess->JobExitTime - selectedProcess->JobArrivalTime);
      SumMetrics[WT] += (selectedProcess->JobExitTime - selectedProcess->JobArrivalTime - selectedProcess->TimeInWaitQueue
         - selectedProcess->TimeInCpu - selectedProcess->TimeInJobQueue);
      
      // since job is done enqueue it to the exit queue
      EnqueueProcess(EXITQUEUE, selectedProcess);

      // update NumberofJobs
      NumberofJobs[THGT]++;
      NumberofJobs[WT]++;
    }
    else {
      // if RR set CpuBurstTime to the quantum
      if (PolicyNumber == RR) {
        selectedProcess->CpuBurstTime = Quantum;
      }

      // if remaining time is less then the burst time set the burst time to the remaining time
      if (selectedProcess->RemainingCpuBurstTime < selectedProcess->CpuBurstTime) {
        selectedProcess->CpuBurstTime = selectedProcess->RemainingCpuBurstTime;
      }

      // if burst time is greater than the job duration left over then set that
      if (selectedProcess->TotalJobDuration - selectedProcess->TimeInCpu < selectedProcess->CpuBurstTime) {
        selectedProcess->CpuBurstTime = selectedProcess->TotalJobDuration - selectedProcess->TimeInCpu;
      }
      
      // put the process on the cpu
      OnCPU(selectedProcess, selectedProcess->CpuBurstTime);
      // update TimeInCpu
      selectedProcess->TimeInCpu += selectedProcess->CpuBurstTime;
      // update RemainingCpuBurstTime
      selectedProcess->RemainingCpuBurstTime = (selectedProcess->CpuBurstTime - selectedProcess->RemainingCpuBurstTime);
      // put process on the running queue
      EnqueueProcess(RUNNINGQUEUE, selectedProcess);
      // update metrics
      SumMetrics[CBT] += selectedProcess->CpuBurstTime;
    }
  }
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run when a job is added to the Job Queue    *
\***********************************************************************/
void NewJobIn(ProcessControlBlock whichProcess){
  ProcessControlBlock *NewProcess;
  /* Add Job to the Job Queue */
  NewProcess = (ProcessControlBlock *) malloc(sizeof(ProcessControlBlock));
  memcpy(NewProcess,&whichProcess,sizeof(whichProcess));
  EnqueueProcess(JOBQUEUE,NewProcess);
  DisplayQueue("Job Queue in NewJobIn",JOBQUEUE);
  LongtermScheduler(); /* Job Admission  */
}


/***********************************************************************\                                               
* Input : None                                                         *                                                
* Output: None                                                         *                
* Function:                                                            *
* 1) BookKeeping is called automatically when 250 arrived              *
* 2) Computes and display metrics: average turnaround  time, throughput*
*     average response time, average waiting time in ready queue,      *
*     and CPU Utilization                                              *                                                     
\***********************************************************************/
void BookKeeping(void){
  double end = Now(); // Total time for all processes to arrive
  Metric m;

  // Compute averages and final results
  // ........

  printf("\n********* Processes Managemenent Numbers ******************************\n");
  printf("Policy Number = %d, Quantum = %.6f   Show = %d\n", PolicyNumber, Quantum, Show);
  printf("Number of Completed Processes = %d\n", NumberofJobs[THGT]);
  printf("ATAT=%f   ART=%f  CBT = %f  T=%f AWT=%f\n", 
	 SumMetrics[TAT]/NumberofJobs[THGT], SumMetrics[RT]/NumberofJobs[RT], SumMetrics[CBT]/end, 
	 NumberofJobs[THGT]/end, SumMetrics[WT]/NumberofJobs[WT]);

  exit(0);
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: Decides which processes should be admitted in Ready Queue   *
*           If enough memory and within multiprogramming limit,         *
*           then move Process from Job Queue to Ready Queue             *
\***********************************************************************/
void LongtermScheduler(void){
  ProcessControlBlock *currentProcess = DequeueProcess(JOBQUEUE);
  while (currentProcess) {
    currentProcess->TimeInJobQueue = Now() - currentProcess->JobArrivalTime; // Set TimeInJobQueue
    currentProcess->JobStartTime = Now(); // Set JobStartTime
    EnqueueProcess(READYQUEUE,currentProcess); // Place process in Ready Queue
    currentProcess->state = READY; // Update process state
    currentProcess = DequeueProcess(JOBQUEUE);
  }
}


/***********************************************************************\
* Input : None                                                          *
* Output: TRUE if Intialization successful                              *
\***********************************************************************/
Flag ManagementInitialization(void){
  Metric m;
  for (m = TAT; m < MAXMETRICS; m++){
     NumberofJobs[m] = 0;
     SumMetrics[m]   = 0.0;
  }
  return TRUE;
}
