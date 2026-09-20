#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "process.h"

void cpu(int n, int m, int *resources, Tnode *ready, Plist *waiting);
void print(char *s, ...);
void flush(void);

int main()
{
    int n;
    scanf("%d", &n);
    int m;
    scanf("%d", &m);
    int resources[m+1];
    for (int i = 0; i < m; i++)
        scanf("%d", &resources[i]);
    if (m == 0)
        scanf("%d", resources);
    int PC;
    scanf("%d", &PC);
    int PS;
    scanf("%d", &PS);
    Tnode *ready = NULL;
    for (int i = 0; i < n; i++)
        ready = tadd(ready, read_process_info(i));
    cpu(n, m, resources, ready, NULL);
    flush();
    return 0;
}

#define DEADLOCK_WAIT 10

void cpu(int n, int m, int *available, Tnode *ready, Plist *waiting)
{
    int work[m]; /* Number of available instances of each resource type */
    int allocation[n][m];
    int finish[n];
    memset(allocation, 0, sizeof(allocation));
    int request[n][m];
    memset(request, 0, sizeof(request));
    int t = 0;
    while (ready != NULL) {
        Tnode **node = &ready;
        while ((*node)->left != NULL)
            node = &(*node)->left;
        Process *process = (*node)->process;
        Tnode *tmp = (*node)->right;
        free(*node);
        *node = tmp;
        
        for (int i = 0; i < m; i++)
            if (request[process->pid][i] > available[i]) {
                print("DID NOT EXECUTE %d\n", process->pid);
                waiting = palloc(t, (DEADLOCK_WAIT - t%DEADLOCK_WAIT), process, waiting);
                goto out;
            }
        for (int i = 0; i < m; i++) {
            if (request[process->pid][i] > 0) {
                print("GIVE %d %d %d %d\n", process->pid, request[process->pid][i], i, t);
                allocation[process->pid][i] += request[process->pid][i]; //available[i];
                available[i] -= request[process->pid][i];
                request[process->pid][i] = 0;
            }
        }

        while (process->pc < process->ic) {
            switch (process->instructions[process->pc].type) {
                case Run:
                    print("EXECUTE %d %d %d\n", process->pid, t, t + process->instructions[process->pc].args[0]);
                    t += process->instructions[process->pc++].args[0];
                break;

                case Allocate:
                    if (process->instructions[process->pc].args[0] <= available[process->instructions[process->pc].args[1]]) {
                        available[process->instructions[process->pc].args[1]] -= process->instructions[process->pc].args[0];
                        allocation[process->pid][process->instructions[process->pc].args[1]] += process->instructions[process->pc].args[0];
                        print("GIVE %d %d %d %d\n", process->pid, process->instructions[process->pc].args[0], process->instructions[process->pc].args[1], t);
                        process->pc++;
                    } else {
                        request[process->pid][process->instructions[process->pc].args[1]] = process->instructions[process->pc].args[0];
                        waiting = palloc(t, (DEADLOCK_WAIT - t%DEADLOCK_WAIT)%DEADLOCK_WAIT, process, waiting);
                        process->pc++; /* Testing */
                        goto out;
                    }
                break;

                case Free:
                    available[process->instructions[process->pc].args[1]] += process->instructions[process->pc].args[0];
                    allocation[process->pid][process->instructions[process->pc].args[1]] -= process->instructions[process->pc].args[0];
                    print("TAKE %d %d %d %d\n", process->pid, process->instructions[process->pc].args[0], process->instructions[process->pc].args[1], t);
                    process->pc++;
                break;

                case Sleep:
                    print("WAIT %d %d %d\n", process->pid, t, t + process->instructions[process->pc].args[0]);
                    waiting = palloc(t, process->instructions[process->pc++].args[0], process, waiting);
                    goto out;
                break;
            }
        }

        if (process->pc >= process->ic) {
            for (int i = 0; i < m; i++) {
                if (allocation[process->pid][i] > 0) {
                    available[i] += allocation[process->pid][i];
                    print("EMERGENCY TAKE %d %d %d %d\n", process->pid, allocation[process->pid][i], i, t);
                    allocation[process->pid][i] = 0;
                }
            }
            free(process->instructions);
            free(process);
        }

        out:

        


        if (1 /*t%DEADLOCK_WAIT == 0*/) {
            int deadlocked;
            memcpy(work, available, sizeof(work));
            memset(finish, 0, sizeof(finish));
            do {
                for (int i = 0; i < n; i++) {
                    if (finish[i] == 1)
                        continue;
                    int j = 0;
                    while (j < m && request[i][j] <= work[j])
                        j++;
                    if (j < m)
                        continue;
                    for (int j = 0; j < m; j++)
                        work[j] += allocation[i][j];
                    finish[i] = 1;
                    i = 0;
                }
                deadlocked = 0;
                for (int i = 0; i < n; i++) {
                    if (finish[i] == 0) {
                        deadlocked = 1;
                        for (int j = 0; j < m; j++) {
                            if (allocation[i][j] > 0) {
                                print("DEADLOCK TAKE %d %d %d %d\n", i, allocation[i][j], j, t);
                                available[j] += allocation[i][j];
                                work[j] += allocation[i][j];
                                request[i][j] += allocation[i][j];
                                allocation[i][j] = 0;
                            }
                        }
                        finish[i] = 1;
                        break;
                    }
                }
            } while (deadlocked);
        }

        if (ready == NULL && waiting != NULL) {
            int earliest_finish_time = waiting->time_stamp + waiting->sleep_time;
            for (Plist *node = waiting; node != NULL; node = node->next)
                if (node->time_stamp + node->sleep_time < earliest_finish_time)
                    earliest_finish_time = node->time_stamp + node->sleep_time;
            if (t < earliest_finish_time)
                t = earliest_finish_time;
        }

        for (Plist **node_ptr = &waiting; *node_ptr != NULL;) {
            if ((*node_ptr)->time_stamp + (*node_ptr)->sleep_time <= t) {
                ready = tadd(ready, (*node_ptr)->process);
                Plist *tmp = *node_ptr;
                *node_ptr = (*node_ptr)->next;
                free(tmp);
            } else {
                node_ptr = &(*node_ptr)->next;
            }
        }
    }
}

#define BUFUNIT 256

static char *buf = NULL;
static int bufsize = 0;
static int bufp = 0;
static int print_count = 0;

void print(char *s, ...)
{
    va_list parameters;
    va_start(parameters, s);
    char tmp[BUFUNIT];
    if (vsnprintf(tmp, BUFUNIT, s, parameters) >= BUFUNIT)
        printf("Error: String longer than %d\n", BUFUNIT);

    int len = strlen(tmp);

    while (bufp + len >= bufsize)
        buf = realloc(buf, bufsize += BUFUNIT);
    memcpy(buf + bufp, tmp, sizeof(char) * len);
    bufp += len;
    print_count++;
}

void flush(void)
{
    printf("%d\n", print_count);
    fwrite(buf, sizeof(char), bufp, stdout);
    fflush(stdout);
    free(buf);
    buf = NULL;
    bufsize = 0;
    bufp = 0;
}