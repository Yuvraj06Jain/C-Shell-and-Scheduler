#include "functions.h"

volatile sig_atomic_t fg_timedOut = 0;
pid_t fg_pgid = 0;


void sigalarm_handler(int sig){
    if(fg_pgid>0){
        kill(-fg_pgid, SIGTERM);
        fg_timedOut = 1;
    }
}


int fg(int job, int timeout){
    int idx = -1;
    for(int i=0;i<grProCount;i++){
        if(grPro[i].job == job && grPro[i].background){
            idx = i;
            break;
        }
    }

    if(idx == -1){
        printf("resume: no such jobs\n");
        return 1;
    }

    grPro[idx].background = false;

    for(int i=0;i<grPro[idx].numProcs;i++){
        grPro[idx].procs[i].state = "Running";
    }

    printf("%s\n", grPro[idx].fullCommand);

    tcsetpgrp(STDIN_FILENO, grPro[idx].pgid);

    struct sigaction sa, old_sa;
    sa.sa_handler = sigalarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, &old_sa);

    if(timeout != 0){
        alarm(timeout);
    }

    fg_timedOut = 0;
    fg_pgid = grPro[idx].pgid;

    kill(-grPro[idx].pgid, SIGCONT);

    int active = 1;
    while(active) {
        active = 0;
        for (int i = 0; i < grPro[idx].numProcs; i++) {
            if (!strcmp(grPro[idx].procs[i].state, "Running")) {
                active = 1;
                break;
            }
        }
        if (active) {
            pause();
        }
    }

    if(timeout != 0){
        alarm(0); // Cancel alarm!
    }

    tcsetpgrp(STDIN_FILENO, getpgrp());

    if(fg_timedOut){
        printf("resume: job timed out\n");
    }

    sigaction(SIGALRM, &old_sa, NULL);

    return 0;
}


int bg(int job){
    int idx = -1;
    for(int i=0;i<grProCount;i++){
        if(grPro[i].job == job && grPro[i].background){
            idx = i;
            break;
        }
    }

    if(idx == -1){
        printf("resume: no such jobs\n");
        return 1;
    }

    for(int i=0;i<grPro[idx].numProcs;i++){
        grPro[idx].procs[i].state = "Running";
    }

    printf("[%d] + Running  %s\n", grPro[idx].job, grPro[idx].fullCommand);

    kill(-grPro[idx].pgid, SIGCONT);

    return 0;
}

int resume(Node* args, Node* end){
    bool isfg = false; bool isbg = false;
    int timeout = 0; int job;

    Node* temp = args;
    
    if(temp != end && temp->token[0] == '%'){
        char* endptr;
        job = strtol(temp->token + 1, &endptr, 10);
    }
    else{
        printf("resume: invalid syntax\n");
        return 1;
    }

    temp = temp->next;
    if(temp == end){
        printf("resume: invalid syntax\n");
        return 1;
    }

    if(!strcmp(temp->token, "fg")){
        isfg = true;
        temp = temp->next;
        if(temp != end && !strcmp(temp->token, "--timeout")){
            temp = temp->next;

            if(temp != end){
                char* endptr;
                timeout = strtol(temp->token, &endptr, 10);
                temp = temp->next;
                if(temp != end) {
                    printf("resume: invalid syntax\n");
                    return 1;
                }
            }
            else{
                printf("resume: invalid syntax\n");
                return 1;
            }
        }
        else if (temp != end) {
            printf("resume: invalid syntax\n");
            return 1;
        }
    }
    else if(!strcmp(temp->token, "bg")){
        isbg = true;
        temp = temp->next;
        if (temp != end) {
            printf("resume: invalid syntax\n");
            return 1;
        }
    }
    else{
        printf("resume: invalid syntax\n");
        return 1;
    }

    int ret = 0;
    if(isfg){
        ret = fg(job, timeout);
    }
    else if(isbg){
        ret = bg(job);
    }

    return ret;
}
