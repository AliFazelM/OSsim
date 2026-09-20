#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "process.h"

Process *read_process_info(int pid)
{
    Process *process = malloc(sizeof(Process));
    process->pid = pid;
    process->pc = 0;
    scanf("%d", &process->ic);
    process->instructions = malloc(sizeof(Instruction) * process->ic);
    for (int i = 0; i < process->ic; i++) {
        char name[12];
        scanf("%s", name);
        if (strcmp(name, "Run") == 0) {
            process->instructions[i] = (Instruction) {Run};
            scanf("%d", &process->instructions[i].args[0]);
        }
        else if (strcmp(name, "Sleep") == 0) {
            process->instructions[i] = (Instruction) {Sleep};
            scanf("%d", &process->instructions[i].args[0]);
        }
        else if (strcmp(name, "Allocate") == 0) {
            process->instructions[i] = (Instruction) {Allocate};
            scanf("%d %d", &process->instructions[i].args[0], &process->instructions[i].args[1]);
        }
        else if (strcmp(name, "Free") == 0) {
            process->instructions[i] = (Instruction) {Free};
            scanf("%d %d", &process->instructions[i].args[0], &process->instructions[i].args[1]);
        }
        else if (strcmp(name, "Read") == 0) {
            process->instructions[i] = (Instruction) {Read};
            scanf("%d", &process->instructions[i].args[0]);
        }
        else if (strcmp(name, "Write") == 0) {
            process->instructions[i] = (Instruction) {Write};
            scanf("%d", &process->instructions[i].args[0]);
        }
    }
    return process;
}

Tnode *talloc(Process *process)
{
    Tnode *tp = malloc(sizeof(Tnode));
    tp->process = process;
    tp->left = tp->right = NULL;
    return tp;
}

int get_running_time(Process *p)
{
    int tmp = p->pc;
    while (tmp < p->ic && p->instructions[tmp].type != Run)
        tmp++;
    return tmp < p->ic ? p->instructions[tmp].args[0] : 0;
}

Tnode *tadd(Tnode *tp, Process *process)
{
    if (tp == NULL)
        tp = talloc(process);
    else if (get_running_time(tp->process) > get_running_time(process)) /* added an = sign */
        tp->left = tadd(tp->left, process);
    else
        tp->right = tadd(tp->right, process);
    return tp;
}

Plist *palloc(int time_stamp, int sleep_time, Process *process, Plist *next)
{
    Plist *plist_node = malloc(sizeof(Plist));
    plist_node->time_stamp = time_stamp;
    plist_node->sleep_time = sleep_time;
    plist_node->process = process;
    plist_node->next = next;
    return plist_node;
}