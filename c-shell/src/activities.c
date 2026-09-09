#include "functions.h"

int activities(){
    for(int i=0;i<grProCount;i++){
        if(!grPro[i].background) {
            continue;
        }

        int active_procs = 0;
        for(int j=0;j<grPro[i].numProcs;j++){
            if(strcmp(grPro[i].procs[j].state, "Completed") != 0 && strcmp(grPro[i].procs[j].state, "Killed") != 0){
                active_procs++;
            }
        }

        if(active_procs > 0){
            printf("[%d] pgid %d\n", grPro[i].job, (int)grPro[i].pgid);

            for(int j=0;j<grPro[i].numProcs;j++){
                if(strcmp(grPro[i].procs[j].state, "Completed") != 0 && strcmp(grPro[i].procs[j].state, "Killed") != 0){
                    printf("%d  %s  %s\n", (int)grPro[i].procs[j].pid, grPro[i].procs[j].cmdName, grPro[i].procs[j].state);
                }
            }
        }
    }

    return 0;
}