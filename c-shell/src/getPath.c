#include "functions.h"

int getPathDirs(char*** pathDirs){
    char* path = strdup(getenv("PATH"));

    int cap = 2; int idx1 = 0; int len = 100;
    *pathDirs = (char**)malloc(cap * sizeof(char*)); char* temp = (char*)malloc(len * sizeof(char)); int idx = 0;
    for(int i=0;path[i]!='\0';i++){
        if(path[i] == ':'){
            temp[idx] = '\0';
            
            if(idx1 == cap){
                cap = cap * 2;
                *pathDirs = realloc(*pathDirs, cap * sizeof(char*));
            }
            (*pathDirs)[idx1++] = strdup(temp);

            idx = 0;
            len = 100;
        }
        else{
            if(idx == len-1){
                len = len * 2;
                temp = realloc(temp, len * sizeof(char));
            }

            temp[idx++] = path[i];
        }
    }

    temp[idx] = '\0';
    if(idx1 == cap){
        cap = cap + 1;
        *pathDirs = (char**)realloc(*pathDirs, cap * sizeof(char*));
    }
    (*pathDirs)[idx1++] = strdup(temp);

    free(temp);
    return idx1;
}
