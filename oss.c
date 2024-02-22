#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <getopt.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_PROC 10

struct systemClock{
    int seconds;
    int nanoseconds;
};

struct PCB {
    pid_t pid;
    struct systemClock launchTime;
    bool isFree;
};

struct PCB processTable[MAX_PROC];

void handle_alarm(int sig){
    for(int i = 0; i < MAX_PROC; ++i){
        if(!processTable[i].isFree){
            kill(processTable[i].pid, SIGKILL); //kill each child 
            processTable[i].isFree = true; 
        }
    }
    printf("Terminated after 60 seconds.");
    exit(0);
}

int main(int argc, char*argv[]){
	
	//Initialize processTable
    for(int i = 0; i < MAX_PROC; ++i)
        processTable[i].isFree = true;

    //setup alarm signal handler
    signal(SIGALRM, handle_alarm);
    //schedule alarm for 60 seconds
    alarm(60);
    
    // variables for command line parameters
	int proc = 5;
    int simul = 10, timelimit = 2;
    int intervalInMs = 1000;
    int option;
    while ((option = getopt(argc, argv, "hn:s:t:i:")) != -1) {
        switch (option) { 
            case 'h' : // print help
				printf("Usage: ./oss [-h] [-n [proc]] [-s simul] [-t timelimit] [-i intervalInMs]\n"
					   "\t-h\t\tHelp: show this usage message\n"
					   "\t-n proc\tTotal number of children to launch [Default: 5]\n"
					   "\t-s simul\tNumber of children that can run simultaneously\n"
					   "\t-t timelimit\tMaximum time a child process will run [Default: 2]\n"
					   "\t-i intervalInMs\tInterval for launching children processes in milliseconds [Default: 1000]\n");
				exit(EXIT_SUCCESS);
                break;
			case 'n' : // number of total children to launch
                proc = atoi(optarg);
                break;
            case 's' :    // how many children to allow to run simultaneously
                simul = atoi(optarg);
                break;
            case 't' :    // max time child processes will run
                timelimit = atoi(optarg);
                break;
            case 'i' :    // interval for launching children processes
                intervalInMs = atoi(optarg);
                break;
            default:
                // Invalid command line arguments
                // Fill this part
                exit(EXIT_FAILURE);
        }
    }
    
    int shmid = shmget(IPC_PRIVATE, sizeof(struct systemClock), IPC_CREAT | 0666);
	if (shmid < 0) {
		perror("shmget failed in oss");
		exit(EXIT_FAILURE);
	}
    struct systemClock* clock = (struct systemClock*) shmat(shmid, NULL, 0);
    clock->seconds = 0;
    clock->nanoseconds = 0;
    
    srand(time(NULL)); // for generating random numbers
    int i, status, nextLaunchMs = 0;
	int totalProc = 0;
	int timeCounter = 0;
	int runningProc = 0;
    pid_t pid;
    
	while(totalProc < proc) {
		if(runningProc < simul && clock->seconds * 1000 + clock->nanoseconds / 1000000 >= nextLaunchMs) {
			pid = fork();
			if(pid < 0){
				perror("Fork failed");
			}
			if(pid == 0){
				char secStr[10], nsecStr[10];
				int sec = rand() % timelimit + 1;
				int nsec = rand() % 1000000000;
				sprintf(secStr, "%d", sec);
				sprintf(nsecStr, "%d", nsec);
				char shmidStr[10];
				sprintf(shmidStr, "%d", shmid);
				char* args[] = {"./worker", secStr, nsecStr, shmidStr, NULL};
				execv("./worker", args);
			} else {
				// find and update next free slot in processTable
				for(int j = 0; j < MAX_PROC; ++j){
					if(processTable[j].isFree){
						processTable[j].pid = pid;
						processTable[j].launchTime = *clock; // note the time just before the child is launched
						processTable[j].isFree = false;
						break;
					}
				}
				totalProc++;
				nextLaunchMs = clock->seconds * 1000 + clock->nanoseconds / 1000000 + intervalInMs;
			}
			runningProc++;
		}
		// make clock increment happen after child process is created
		usleep(1000); //simulate 1 ms pass in system clock
		clock->nanoseconds += 1000000;
		timeCounter += 1000; // move this line here

		if(clock->nanoseconds >= 1000000000){
			clock->seconds += 1;
			clock->nanoseconds -= 1000000000;
		}
		// Handle child process termination
		pid_t pid = waitpid(-1, &status, WNOHANG);
		if(pid > 0){
			if(WIFEXITED(status)){
				for(int j = 0; j < MAX_PROC; ++j){
					if(processTable[j].pid == pid){
						processTable[j].isFree = true;
						runningProc--; // decrease running processes count as a child has terminated
						break;
					}
				}
			}
		}

		// Print process table every 0.5 seconds.
		if(timeCounter >= 500){
			printf("OSS PID:%d SysClockS: %d SysclockNano: %d\n", getpid(), clock->seconds, clock->nanoseconds);
			printf("Process Table:\n");
			printf("Entry Occupied PID StartS StartN\n");
			for(int j = 0; j < MAX_PROC; ++j){
				printf("%d %d %d %d %d\n", j, !processTable[j].isFree, processTable[j].pid, 
					processTable[j].launchTime.seconds, processTable[j].launchTime.nanoseconds);
			}
			timeCounter = 0; // reset counter
		}
    }
    
    while((pid = wait(&status)) > 0); //wait for all child processes to terminate

    shmdt(clock);
    shmctl(shmid, IPC_RMID, NULL); // destroy shared memory segment
    return 0;
}