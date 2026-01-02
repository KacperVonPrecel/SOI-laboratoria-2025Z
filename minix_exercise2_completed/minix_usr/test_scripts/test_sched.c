#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lib.h>
#include <sys/wait.h>
#include <time.h>
#include <minix/callnr.h>
#include <signal.h>
#include <string.h>


#define TEST_DURATION 25  
#define NUM_RR_PROCS   3     
#define NUM_AGING_PROCS 3    
#define NUM_SJF_PROCS  3     

typedef struct {
    pid_t pid;
    int group;
    int estimate;
    char name[32];
} test_proc_t;

test_proc_t procs[NUM_RR_PROCS + NUM_AGING_PROCS + NUM_SJF_PROCS];
int proc_count = 0;

volatile sig_atomic_t start_work = 0;


void start_handler(int sig) {
    start_work = 1;
}

int setgroup(group, estimate)
int group;
int estimate;
{
    message m;
    m.m1_i1 = 0;
    m.m1_i2 = group;
    m.m1_i3 = estimate;
    return _syscall(MM_PROC_NR, SETGROUP, &m);
}

void child_process(group, estimate, name)
int group;
int estimate;
const char* name;
{
    int work_iterations;
    int i, j;
    volatile long x;
    int work_amount;
    
    signal(SIGUSR1, start_handler);
    
    if (setgroup(group, estimate) < 0) {
        perror("setgroup failed");
        _exit(1);
    }
    
    printf("Child %s (G%d, est=%d) starting, PID=%d\n", 
           name, group, estimate, getpid());
        
    while (!start_work) {
        pause(); 
    }

    switch(group) {
        case 1:
            printf("[%s] RR process - mixed CPU/IO\n", name);
            for (i = 0; i < 15; i++) {
                printf("[%s] RR iteration %d/15\n", name, i+1);
                
                /* Do some CPU work */
                x = 0;
                for (j = 0; j < 2000000; j++) {
                    x += j * j;
                }
                
                /* Sometimes yield (like I/O) */
                
                if (i % 5 == 0) {
                    sleep(1);
                }
                
            }
            break;
            
        case 2:
            printf("[%s] Aging process - doing work\n", name);
            if (strcmp(name, "MIXED-AGING") != 0) {
                /* I/O-like */
                for (i = 0; i < 20; i++) {
                    printf("[%s] I/O cycle %d/20\n", name, i+1);
                    sleep(1);
                }
            } else {
                /* Mixed mode */
                for (i = 0; i < 5; i++) {
                    printf("[%s] Mixed work %d/5\n", name, i+1);
                    x = 0;
                    for (j = 0; j < 5000000; j++) {
                        x += j * j;
                    }
                    sleep(1);
                }
            }
            break;
            
        case 3:
            printf("[%s] SJF estimated %d units\n", name, estimate);
            
            /* Actually DO work proportional to estimate */
            for (i = 0; i < estimate; i++) {
                printf("[%s] SJF unit %d/%d\n", name, i+1, estimate);
                
                /* Variable work based on estimate */
                if (estimate > 10) {
                    work_amount = 1000000;  /* Long jobs have smaller units */
                } else {
                    work_amount = 3000000;  /* Short jobs have larger units */
                }
                
                x = 0;
                for (j = 0; j < work_amount; j++) {
                    x += j * j;
                }
                
                /* Some SJF processes might do I/O */
                if (i % 2 == 0 && estimate > 5) {
                    sleep(1);
                }
            }
            break;
    }
    printf("Child %s (G%d) finished\n", name, group);
    _exit(0);
}

/* ========== TESTS ========== */
/* Test 1: Group hierarchy */
void test_hierarchy() {
    pid_t pid_g1_1, pid_g1_2, pid_g2_1, pid_g2_2, pid_g3_1, pid_g3_2;
    
    printf("\n=== TEST 1: HIERARCHY G1 > G2 > G3 ===\n");

    pid_g1_1 = fork();
    if (pid_g1_1 == 0) { child_process(1, 0, "RR-ONE"); _exit(0); }
    
    pid_g1_2 = fork();
    if (pid_g1_2 == 0) { child_process(1, 0, "RR-TWO"); _exit(0); }

    sleep(2);

    pid_g2_1 = fork();
    if (pid_g2_1 == 0) { child_process(2, 5, "IO-AGING"); _exit(0); }
    
    pid_g2_2 = fork();
    if (pid_g2_2 == 0) { child_process(2, 3, "MIXED-AGING"); _exit(0); }
    
    sleep(2);

    pid_g3_1 = fork();
    if (pid_g3_1 == 0) { child_process(3, 4, "SJF-SHORT"); _exit(0); }
    
    pid_g3_2 = fork();
    if (pid_g3_2 == 0) { child_process(3, 8, "SJF-LONG"); _exit(0); }
    
    
    
    printf("\nWhat should happen is that the completion should go like this:\n");
    printf("Procs from G1 >> Procs from G2 >> Procs from G3\n");
    printf("However, sometimes where no procs are ready from upper prirority\n");
    printf("Lower priority procs get time, which is fine\n");
    
    printf("Monitoring for %d seconds...\n", TEST_DURATION);
    
    sleep(5);   
    
    printf("\n--- STARTING ALL PROCESSES ---\n");
    
    kill(pid_g1_1, SIGUSR1);
    kill(pid_g1_2, SIGUSR1);
    kill(pid_g2_1, SIGUSR1);
    kill(pid_g2_2, SIGUSR1); 
    kill(pid_g3_1, SIGUSR1);
    kill(pid_g3_2, SIGUSR1);
  
    sleep(TEST_DURATION);
    
    kill(pid_g1_1, SIGKILL);
    kill(pid_g1_2, SIGKILL);
    kill(pid_g2_1, SIGKILL);
    kill(pid_g2_2, SIGKILL);
    kill(pid_g3_1, SIGKILL);
    kill(pid_g3_2, SIGKILL);
   
    wait(NULL); wait(NULL); wait(NULL);
    wait(NULL); wait(NULL); wait(NULL);
    
    printf("\n=== TEST COMPLETE ===\n");
}

void test_round_robin() {
    int i = 0;
    pid_t pids[NUM_RR_PROCS];
    char names[NUM_RR_PROCS][32];
    start_work = 0;

    printf("=== TEST 2: ROUND ROBIN (Group 1) ===\n");
    
    for (i = 0; i < NUM_RR_PROCS; i++) {
        sprintf(names[i], "RR-%d", i+1);
        pids[i] = fork();
        if (pids[i] == 0) {
            child_process(1, 0, names[i]);
            _exit(0);
        }
    }
    
    printf("Created %d RR processes. Monitoring...\n", NUM_RR_PROCS);
    
    printf("\n--- STARTING ALL PROCESSES ---\n");
    for (i = 0; i < 3; i++) {
        kill(pids[i], SIGUSR1);
    }
    sleep(20);
    
    for (i = 0; i < NUM_RR_PROCS; i++) {
        kill(pids[i], SIGKILL);
        wait(NULL);
    }
    
    printf("Test 2 complete. RR processes should have interleaved.\n");
}

void test_aging() {
    int i = 0;
    pid_t io_proc_1, io_proc_2, mixed_proc;
    start_work = 0;
    
    printf("s=== TEST 3: AGING (Group 2) ===\n");
    
    io_proc_1 = fork();
    if (io_proc_1 == 0) {
    	child_process(2, 8, "IO-AGING-ONE");
    	_exit(0);
    }
    
    mixed_proc = fork();
    if (mixed_proc == 0) {
    	child_process(2, 5, "MIXED-AGING");
    	_exit(0);
    }
    
    sleep(2);
    
    io_proc_2 = fork();
    if (io_proc_2 == 0) {
    	child_process(2, 8, "IO-AGING-TWO");
    	_exit(0);
    }
    
    printf("All processes created and waiting\n\n");
    sleep(1);
    
    printf("\n=== OBSERVING FOR 20 SECONDS ===\n");
    printf("In terms of age:\n");
    printf("At start it will be IO-PROC-ONE > MIXED-PROC > IO-PROC-TWO\n");
    printf("After that it will interlace, because\n");
    printf("when proc is picked its age is set to 0\n");
    printf("So later order should be like: MIXED-PROC >> IO-PROC-TWO >> IO-PROC-ONE\n");
    printf("And so on till one of them finishes the job.\n\n");
    
    sleep(5);
    
    printf("\n--- STARTING ALL PROCESSES ---\n");
    kill(io_proc_1, SIGUSR1);
    kill(mixed_proc, SIGUSR1);
    kill(io_proc_2, SIGUSR1);
    
    
    for (i = 0; i < 4; i++) {
        sleep(5);
        printf("[Monitor] %d seconds elapsed...\n", (i+1)*5);
    }
    
    kill(io_proc_1, SIGKILL);
    kill(mixed_proc, SIGKILL);
    kill(io_proc_2, SIGKILL);
    wait(NULL); wait(NULL); wait(NULL);
    
    printf("\n=== TEST COMPLETE ===\n");
}


void test_sjf() {
    int estimates[] = {1, 10, 2, 5};  
    pid_t pids[4];
    int i = 0;
    char name[32];
    start_work = 0;

    printf("=== TEST 4: SJF (Group 3) ===\n");
    
    for (i = 0; i < 4; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            sprintf(name, "SJF-%d-est%d", i+1, estimates[i]);
            child_process(3, estimates[i], name);
            _exit(0);
        }
        procs[i].pid = pids[i];
        printf("Created process %s with PID %d\n", name, pids[i]);
    }    
    
    printf("Created 3 SJF processes with estimates 1, 10, 2, 5\n");
    printf("They should run in order: 1, 2, 5, 10\n");
    printf("Observing for %d seconds...\n", 15);
    sleep(2);
    
    printf("\n--- STARTING ALL PROCESSES ---\n");
    for (i = 0; i < 4; i++) {
        kill(pids[i], SIGUSR1);
    }
    
    sleep(15);
    
    for (i = 0; i < 4; i++) {
        kill(pids[i], SIGKILL);
        wait(NULL);
    }
    
    printf("Test 4 complete. Shortest jobs should have run first.\n");
}

int main(int argc, char *argv[]) {
    int test_num;
    printf("=== TEST SUITE FOR CUSTOM SCHEDULER ===\n");
    printf("Groups: 1=RR, 2=Aging, 3=SJF\n\n");
    
    if (argc > 1) {
        test_num = atoi(argv[1]);
        switch(test_num) {
            case 1: test_hierarchy(); break;
            case 2: test_round_robin(); break; 
            case 3: test_aging(); break;
            case 4: test_sjf(); break;
            default: printf("Unknown test number\n");
        }
    } else {    
    	test_hierarchy();
    	test_round_robin();
    	test_aging();				  
        test_sjf();
    }
    
    printf("\n=== ALL TESTS COMPLETED ===\n");
    return 0;
}


