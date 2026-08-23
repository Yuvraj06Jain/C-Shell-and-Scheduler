#include "functions.h"

int pathCheck(char* fileName, char** pathDirs, int lenPathDirs){
    int res = 0;

    for(int i = 0; i<lenPathDirs; i++){
        int len = strlen(pathDirs[i]) + strlen(fileName) + 2;
        
        char* fullPath = (char*)malloc(len * sizeof(char));
        sprintf(fullPath, "%s/%s", pathDirs[i], fileName);

        struct stat st;
        if(access(fullPath, X_OK) == 0 && stat(fullPath, &st) == 0 && S_ISREG(st.st_mode)){
            char* absPath = realpath(fullPath, NULL);

            if(absPath != NULL){
                printf("%s\n", absPath);
                free(absPath);
                res = 1;
            }
        }

        free(fullPath);
    }

    return res;
}

int currDirCheck(char* fileName){
    if(access(fileName, F_OK | X_OK) != 0){
        return 0;
    }

    char* absPath = realpath(fileName, NULL);
    if(absPath == NULL){
        return 0;
    }

    struct stat st;
    if(stat(fileName, &st) != 0 || !S_ISREG(st.st_mode)){
        return 0;
    }

    printf("%s\n",absPath);
    free(absPath);
    return 1;
}

int main(int argc, char* argv[]){

    if(argc == 1){
        printf("locate: invalid syntax\n");
        return 1;
    }

    char** pathDirs = NULL;
    int lenPathDirs = getPathDirs(&pathDirs);
    
    for(int i=1;i<argc;i++){
        char* token = argv[i];

        int x = currDirCheck(token);
        x = pathCheck(token, pathDirs, lenPathDirs) || x;

        if(!x){
            printf("locate: command not found (%s)\n", token);
        }

    }

    for(int i = 0; i<lenPathDirs; i++){
        free(pathDirs[i]);
    }
    free(pathDirs);

    return 0;
}