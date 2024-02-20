#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <getopt.h>
#include <stdbool.h>

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

int main(int argc, char*argv[]){
    
    // variables for command line parameters
    int simul = 10, timelimit = 2;
    int intervalInMs = 1000;
    int option;
    while ((option = getopt(argc, argv,"hs:t:i:n:")) != -1) {
        switch (option) { 
            case 'h' : // print help
				printf("Usage: ./oss [-h] [-s simul] [-t timelimit] [-i intervalInMs]\n"
					   "\t-h\t\tHelp: show this usage message\n"
					   "\t-s simul\tNumber of processes to be spawned [Default: 10]\n"
					   "\t-t timelimit\tMaximum time a child process will run [Default: 2]\n"
					   "\t-i intervalInMs\tInterval for launching children processes in milliseconds [Default: 1000]\n");
				exit(EXIT_SUCCESS);
                break;
            case 's' :    // number of processes to spawn
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
    pid_t pid;
    
    for(i = 0; i < simul; ++i) {
        if(clock->seconds * 1000 + clock->nanoseconds / 1000000 >= nextLaunchMs) {
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
                        processTable[j].launchTime = *clock;
                        processTable[j].isFree = false;
                        break;
                    }
                }
                nextLaunchMs = clock->seconds * 1000 + clock->nanoseconds / 1000000 + intervalInMs;
            }
        } 
        usleep(1000); // simulate 1 ms pass in system clock
        clock->nanoseconds += 1000000;
        if(clock->nanoseconds >= 1000000000){
            clock->seconds += 1;
            clock->nanoseconds -= 1000000000;
        }
    }
    
    while((pid = wait(&status)) > 0); //wait for all child processes to terminate

    shmdt(clock);
    shmctl(shmid, IPC_RMID, NULL); // destroy shared memory segment
    return 0;
}