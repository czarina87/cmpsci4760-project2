#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/types.h>

struct systemClock{
    int seconds;
    int nanoseconds;
};

int main(int argc, char* argv[]) {
    int shmid;
    struct systemClock* clock;

    shmid = atoi(argv[3]);
    if (shmid < 0) {
        perror("shmget failed in worker");
        exit(EXIT_FAILURE);
    }

    clock = (struct systemClock*) shmat(shmid, NULL, 0);
    if (clock == (void *) -1) {
        perror("shmat failed in worker");
        exit(EXIT_FAILURE);
    }

    if (argc < 3) {
        printf("Not enough arguments\n");
        return 1;
    }

    int targetSecs = atoi(argv[1]);
    int targetNanos = atoi(argv[2]);
    int systemSecs = clock->seconds;
    int systemNanos = clock->nanoseconds;
    int secsPassed = 0;
    targetSecs += systemSecs;
    targetNanos += systemNanos;

    if (targetNanos >= 1000000000) {
        targetSecs += 1;
        targetNanos -= 1000000000;
    }

    while(1) {
        if(clock->seconds >= targetSecs && clock->nanoseconds >= targetNanos) {
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d\n--Terminating\n",
                   getpid(), getppid(), clock->seconds, clock->nanoseconds, targetSecs, targetNanos);
            break;
        }

        if(clock->seconds > systemSecs) {
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d\n--%d seconds have passed since starting\n",
                   getpid(), getppid(), clock->seconds, clock->nanoseconds, targetSecs, targetNanos, ++secsPassed);
            systemSecs = clock->seconds;
        }
    }
    shmdt(clock);
    
    return 0;
}