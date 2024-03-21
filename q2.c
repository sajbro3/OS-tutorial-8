#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>

#define MAX_NAME_LENGTH 256
#define MEMORY 1024

typedef struct {
    char name[MAX_NAME_LENGTH];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    bool suspended;
} proc;

typedef struct {
    proc data[MEMORY];
    int front, rear;
} Queue;

void initQueue(Queue *q) {
    q->front = q->rear = -1;
}

bool isEmpty(Queue *q) {
    return q->front == -1;
}

void push(Queue *q, proc p) {
    if (q->rear == MEMORY - 1) {
        printf("Queue overflow\n");
        return;
    }
    if (q->front == -1) {
        q->front = q->rear = 0;
    } else {
        q->rear++;
    }
    q->data[q->rear] = p;
}

proc pop(Queue *q) {
    proc p;
    if (isEmpty(q)) {
        printf("Queue underflow\n");
        exit(EXIT_FAILURE);
    }
    p = q->data[q->front];
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front++;
    }
    return p;
}

void executeProcess(proc *p, int *avail_mem) {
    printf("Executing process: %s, Priority: %d, PID: %d, Memory: %d, Runtime: %d\n", p->name, p->priority, p->pid, p->memory, p->runtime);
    char *args[] = {p->name, NULL}; // Construct argument array
    p->pid = fork();
    if (p->pid == 0) {
        // Child process
        execv(p->name, args); // Pass the argument array to execv
        exit(EXIT_SUCCESS);
    } else if (p->pid < 0) {
        // Error in forking
        printf("Error forking process.\n");
    }
}

void suspendProcess(proc *p) {
    kill(p->pid, SIGTSTP);
    p->suspended = true;
}

void resumeProcess(proc *p) {
    kill(p->pid, SIGCONT);
    p->suspended = false;
}

int main() {
    FILE *fp;
    Queue priority, secondary;
    proc process;
    int avail_mem[MEMORY] = {0}; // Initialize all memory to 0
    char filename[] = "processes_q2.txt";

    initQueue(&priority);
    initQueue(&secondary);

    // Read processes from file
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(EXIT_FAILURE);
    }

    while (fscanf(fp, "%[^,],%d,%d,%d\n", process.name, &process.priority, &process.memory, &process.runtime) != EOF) {
        process.pid = 0;
        process.address = 0;
        process.suspended = false;

        if (process.priority == 0) {
            push(&priority, process);
        } else {
            push(&secondary, process);
        }
    }

    fclose(fp);

    // Execute processes from priority queue
    while (!isEmpty(&priority)) {
        proc current_process = pop(&priority);
        executeProcess(&current_process, avail_mem);
        // Mark memory as used
        for (int i = current_process.address; i < current_process.address + current_process.memory; i++) {
            avail_mem[i] = 1;
        }
        // Wait for process to terminate
        waitpid(current_process.pid, NULL, 0);
        // Free memory used
        for (int i = current_process.address; i < current_process.address + current_process.memory; i++) {
            avail_mem[i] = 0;
        }
    }

    // Execute processes from secondary queue
    while (!isEmpty(&secondary)) {
        proc current_process = pop(&secondary);
        // Check if there is enough memory
        bool enough_memory = true;
        for (int i = 0; i < MEMORY; i++) {
            if (avail_mem[i] == 0) {
                enough_memory = false;
                break;
            }
        }
        if (enough_memory) {
            executeProcess(&current_process, avail_mem);
            // Mark memory as used
            for (int i = current_process.address; i < current_process.address + current_process.memory; i++) {
                avail_mem[i] = 1;
            }
            // If process is suspended, resume it
            if (current_process.suspended) {
                resumeProcess(&current_process);
            }
            sleep(1); // Run for 1 second
            suspendProcess(&current_process);
            current_process.runtime--;
            if (current_process.runtime > 0) {
                push(&secondary, current_process); // Add back to secondary queue
            }
        } else {
            push(&secondary, current_process); // Push back to queue if not enough memory
        }
    }

    // Terminate remaining processes in secondary queue with 1 second runtime
    while (!isEmpty(&secondary)) {
        proc current_process = pop(&secondary);
        if (current_process.runtime == 1) {
            executeProcess(&current_process, avail_mem);
            // Mark memory as used
            for (int i = current_process.address; i < current_process.address + current_process.memory; i++) {
                avail_mem[i] = 1;
            }
            // Wait for process to terminate
            waitpid(current_process.pid, NULL, 0);
            // Free memory used
            for (int i = current_process.address; i < current_process.address + current_process.memory; i++) {
                avail_mem[i] = 0;
            }
        }
    }

    return 0;
}
