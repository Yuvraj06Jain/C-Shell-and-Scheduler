#include "functions.h"

int ping(Node* args, Node* end){
    int pid = -1; int job = -1;
    int signal_number = -1;

    char* signal_str = NULL;
    char* tar = NULL;

    Node* temp = args;
    if(temp != end){
        tar = temp->token;
        if(temp->token[0] == '%'){
            char* endptr;
            job = strtol(temp->token+1, &endptr, 10);
            if(job < 0 || *endptr != '\0'){
                printf("ping: invalid syntax\n");
                return 1;
            }
        }
        else{
            char* endptr;
            pid = strtol(temp->token, &endptr, 10);
            if(pid < 0 || *endptr != '\0'){
                printf("ping: invalid syntax\n");
                return 1;
            }
        }
    }
    else{
        printf("ping: invalid syntax\n");
        return 1;
    }

    temp = temp->next;
    if(temp != end){
        char* endptr;
        signal_number = strtol(temp->token, &endptr, 10);
        signal_str = temp->token;
        
        if(signal_number < 0 || *endptr != '\0'){
            printf("ping: invalid syntax\n");
            return 1;
        }
    }
    else {
        printf("ping: invalid syntax\n");
        return 1;
    }

    int sig = signal_number % 64;

    bool found = false;
    for(int i = 0; i < grProCount; i++){

        int active_procs = 0;
        for(int j = 0; j < grPro[i].numProcs; j++){
            if(strcmp(grPro[i].procs[j].state, "Completed") != 0 && strcmp(grPro[i].procs[j].state, "Killed") != 0){
                active_procs++;
            }
        }
        
        if (active_procs == 0){
            continue;
        }

        if(job > 0){
            if(grPro[i].job == job && grPro[i].background){
                found = true;
                kill(-grPro[i].pgid, sig);
                break;
            }
        }
        else{
            for(int j = 0; j < grPro[i].numProcs; j++){
                if(pid == grPro[i].procs[j].pid){
                    if(strcmp(grPro[i].procs[j].state, "Completed") != 0 && strcmp(grPro[i].procs[j].state, "Killed")){
                        found = true;
                        kill(pid, sig);
                        break;
                    }
                }
            }
            if(found) break;
        }
    }

    if(!found){
        printf("ping: no such process found\n");
    }
    else{
        printf("Sent signal %s to %s\n", signal_str, tar);
    }
    
    return 0;
}


// Assumption/Implementation Detail: Our shell strictly adheres to the requirement to report background process terminations immediately. When a process is killed (e.g., via ping %job 9), the SIGCHLD handler instantly reaps the process, marks its state as "Killed", and prints the completion message. Consequently, any subsequent ping to that process will correctly report "no such process found", rather than falsely succeeding by sending signals to a lingering zombie process.