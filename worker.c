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

    int targetSeconds = atoi(argv[1]);
    int targetNanos = atoi(argv[2]);
    int systemSecs = clock->seconds;
    int systemNanos = clock->nanoseconds;
    int secsPassed = 0;

    targetSeconds += systemSecs;
    targetNanos += systemNanos;

    printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TargetTermS: %d TargetTermNano: %d\n--Just Starting\n", 
        getpid(), getppid(), systemSecs, systemNanos, targetSeconds, targetNanos);

    while(1) {       
        if(clock->seconds > systemSecs) {
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TargetTermS: %d TargetTermNano: %d\n--%d seconds have passed since starting\n", 
                getpid(), getppid(), clock->seconds, clock->nanoseconds, targetSeconds, targetNanos, ++secsPassed);
            systemSecs = clock->seconds;
        }

        if(clock->seconds >= targetSeconds && clock->nanoseconds >= targetNanos) {
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TargetTermS: %d TargetTermNano: %d\n--Terminating\n",
                getpid(), getppid(), clock->seconds, clock->nanoseconds, targetSeconds, targetNanos);
            break;
        }
    }

    shmdt(clock);
    return 0;
}