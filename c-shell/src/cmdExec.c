#include "functions.h"

int isExecutable(char* path){
    struct stat st;
    if(stat(path, &st) != 0) return 0;
    if(!S_ISREG(st.st_mode)) return 0;
    if(access(path, X_OK) != 0) return 0;
    return 1;
}

void child(char* resolvedPath, char* cmdName, Node* args){
    int argc = 1;
    Node* temp = args;
    while(temp != NULL){
        argc++;
        temp = temp->next;
    }


    char** argv = (char**)malloc((argc + 1) * sizeof(char*));
    argv[0] = cmdName;

    int i = 1;
    temp = args;
    while(temp != NULL){
        argv[i++] = temp->token;
        temp = temp->next;
    }
    argv[argc] = NULL;

    execv(resolvedPath, argv);


    printf("cshell: command not found (%s)\n", cmdName);
    free(argv);
    exit(1);
}

int execute(Node* args){
    char* token = args->token;
    Node* cmdArgs = args->next;

    int onlyPath = 0;
    char* cmdName = token;
    if(token[0] == '%'){
        onlyPath = 1;
        cmdName = token + 1;
    }

    char* resolvedPath = NULL;

    if(strchr(cmdName, '/') != NULL){
        if(isExecutable(cmdName)){
            resolvedPath = strdup(cmdName);
        }
    }
    else{
        if(!onlyPath){
            char* cwdPath = (char*)malloc((strlen(cwd) + strlen(cmdName) + 2) * sizeof(char));
            sprintf(cwdPath, "%s/%s", cwd, cmdName);

            if(isExecutable(cwdPath)) resolvedPath = cwdPath;
            else free(cwdPath);
        }

        if(resolvedPath == NULL){
            char** pathDirs = NULL;
            int lenPathDirs = getPathDirs(&pathDirs);

            for(int i = 0; i < lenPathDirs; i++){
                int len = strlen(pathDirs[i]) + strlen(cmdName) + 2;
                char* fullPath = (char*)malloc(len * sizeof(char));
                sprintf(fullPath, "%s/%s", pathDirs[i], cmdName);

                if(resolvedPath == NULL && isExecutable(fullPath)) resolvedPath = fullPath;
                else free(fullPath);
            }

            for(int i = 0; i < lenPathDirs; i++){
                free(pathDirs[i]);
            }

            free(pathDirs);
        }
    }

    if(resolvedPath == NULL){
        printf("cshell: command not found (%s)\n", cmdName);
        return 1;
    }

    pid_t pid = fork();
    if(pid < 0){
        printf("cshell: fork failed\n");
        free(resolvedPath);
        return 1;
    }
    else if(pid == 0){
        child(resolvedPath, cmdName, cmdArgs);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
    }

    free(resolvedPath);
    return 0;
}