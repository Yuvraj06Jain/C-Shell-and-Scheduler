#include "functions.h"

int activities(){
    for(int i=0;i<bgCount;i++){
        int active_procs = 0;
        for(int j=0;j<bgPro[i].numProcs;j++){
            if(strcmp(bgPro[i].procs[j].state, "Done")){
                active_procs++;
            }
        }

        if(active_procs > 0){
            printf("[%d] pgid %d\n", bgPro[i].job, (int)bgPro[i].pgid);

            for(int j=0;j<bgPro[i].numProcs;j++){
                if(strcmp(bgPro[i].procs[j].state, "Done")){
                    printf(" %d %s %s\n", (int)bgPro[i].procs[j].pid, bgPro[i].procs[j].cmdName, bgPro[i].procs[j].state);
                }
            }
        }
    }

    return 0;
}