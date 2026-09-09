#include "functions.h"

int isExecutable(char* path){
    struct stat st;
    if(stat(path, &st) != 0) return 0;
    if(!S_ISREG(st.st_mode)) return 0;
    if(access(path, X_OK) != 0) return 0;
    return 1;
}


void closeAllStagePipes(int (*pipeFds)[2], int numPipes){
    for(int j = 0; j < numPipes; j++){
        close(pipeFds[j][0]);
        close(pipeFds[j][1]);
    }
}

void child(char* resolvedPath, char* cmdName, char** cmdArgv, int argCount,
           int hasInput, int inFd, int hasOutput, int outFd,
           int hasPrevPipe, int prevReadFd, int hasNextPipe, int nextWriteFd,
           int (*pipeFds)[2], int numPipes, pid_t pgid){
    
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    if(hasPrevPipe) dup2(prevReadFd, STDIN_FILENO);
    if(hasNextPipe) dup2(nextWriteFd, STDOUT_FILENO);

    if(hasInput){
        dup2(inFd, STDIN_FILENO);
        close(inFd);
    }
    if(hasOutput){
        dup2(outFd, STDOUT_FILENO);
        close(outFd);
    }

    closeAllStagePipes(pipeFds, numPipes);

    int argc = argCount + 1;
    char** argv = (char**)malloc((argc + 1) * sizeof(char*));
    argv[0] = cmdName;

    for(int i = 0; i < argCount; i++) argv[i + 1] = cmdArgv[i];
    argv[argc] = NULL;

    setpgid(0, pgid == 0 ? getpid() : pgid);
    execv(resolvedPath, argv);

    printf("cshell: command not found (%s)\n", cmdName);
    free(argv);
    exit(1);
}

int execute(Node* args, Node* end, int background){
    pid_t pgid = 0;
    int retStatus = 0;

    // Making the Full Command Name.
    int fullLen = 0;
    Node* curr = args;
    while(curr != end){
        if(curr->token) fullLen += strlen(curr->token) + 1;
        curr = curr->next;
    }
    char* fullCmdStr = (char*)malloc((fullLen + 1) * sizeof(char));
    fullCmdStr[0] = '\0';
    curr = args;
    while(curr != end){
        if(curr->token){
            strcat(fullCmdStr, curr->token);
            if(curr->next != end) strcat(fullCmdStr, " ");
        }
        curr = curr->next;
    }


    // Gathering the Pipe Stages
    int numStages = 1;
    Node** stages = (Node**)malloc(numStages * sizeof(Node*)); int stageIdx = 0;

    Node* temp = args;
    stages[stageIdx++] = temp;

    while(temp != end){
        if(temp->type == PIPE){
            if (numStages == stageIdx){
                numStages = numStages * 2;
                stages = (Node**)realloc(stages,(numStages + 1) * sizeof(Node*));
            }
            stages[stageIdx++] = temp->next;
        }

        temp = temp->next;
    }
    numStages = stageIdx;

    
    int pipeFds[numStages-1][2];
    if(numStages > 1){
        for(int i = 0; i < numStages - 1; i++){
            pipe(pipeFds[i]);
        }
    }

    pid_t* pids = (pid_t*)malloc(numStages * sizeof(pid_t));
    pid_t* helperPids = (pid_t*)malloc(numStages * 2 * sizeof(pid_t));
    int helperCount = 0;

    for(int i = 0; i < numStages; i++){
        char* token = stages[i]->token;
        Node* cmdArgs = stages[i]->next;

        int onlyPath = 0;
        char* cmdName = token;
        if(token[0] == '%'){
            onlyPath = 1;
            cmdName = token + 1;
        }

        int argCount = 1; int ltCount = 1; int gtCount = 1; int gtgtCount = 1;
        int ai = 0; int li = 0; int gi = 0; int ggi = 0;

        char** cmdArgv = (char**)malloc((argCount > 0 ? argCount : 1) * sizeof(char*));
        char** ltFiles = (char**)malloc((ltCount > 0 ? ltCount : 1) * sizeof(char*));
        char** gtFiles = (char**)malloc((gtCount > 0 ? gtCount : 1) * sizeof(char*));
        char** gtgtFiles = (char**)malloc((gtgtCount > 0 ? gtgtCount : 1) * sizeof(char*));

        // Gathering all the file Redirections and creating the lists
        temp = cmdArgs;
        while(temp != NULL && temp != end && temp->type != PIPE){
            if(temp->type == LT){
                temp = temp->next;
                if(temp == NULL)
                    break;

                if(li == ltCount){
                    ltCount = ltCount * 2;
                    ltFiles = (char**)realloc(ltFiles, ltCount * sizeof(char*));
                }
                ltFiles[li++] = temp->token;

                temp = temp->next;
            }
            else if(temp->type == GT){
                temp = temp->next;
                if(temp == NULL)
                    break;

                if(gi == gtCount){
                    gtCount = gtCount * 2;
                    gtFiles = (char**)realloc(gtFiles, gtCount * sizeof(char*));
                }
                gtFiles[gi++] = temp->token;

                temp = temp->next;
            }
            else if(temp->type == GTGT){
                temp = temp->next;
                if(temp == NULL)
                    break;

                if(ggi == gtgtCount){
                    gtgtCount = gtgtCount * 2;
                    gtgtFiles = (char**)realloc(gtgtFiles, gtgtCount * sizeof(char*));
                }
                gtgtFiles[ggi++] = temp->token;

                temp = temp->next;
            }
            else{
                if(ai == argCount){
                    argCount = argCount * 2;
                    cmdArgv = (char**)realloc(cmdArgv, argCount * sizeof(char*));
                }
                cmdArgv[ai++] = temp->token;

                temp = temp->next;
            }
        }
        ltCount = li; gtCount = gi; gtgtCount = ggi; argCount = ai;


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

                for(int j = 0; j < lenPathDirs; j++){
                    int len = strlen(pathDirs[j]) + strlen(cmdName) + 2;
                    char* fullPath = (char*)malloc(len * sizeof(char));
                    sprintf(fullPath, "%s/%s", pathDirs[j], cmdName);

                    if(resolvedPath == NULL && isExecutable(fullPath)) resolvedPath = fullPath;
                    else free(fullPath);
                }

                for(int j = 0; j < lenPathDirs; j++){
                    free(pathDirs[j]);
                }

                free(pathDirs);
            }
        }

        if(resolvedPath == NULL){
            printf("cshell: command not found (%s)\n", cmdName);
            retStatus = 1;
            pids[i] = -1;
            free(cmdArgv); free(ltFiles); free(gtFiles); free(gtgtFiles);
            continue;
        }

        // Opening the files for file redirections.
        int failed = 0;

        int* ltfds = (int*)malloc((ltCount > 0 ? ltCount : 1) * sizeof(int));
        for(int j = 0; j < ltCount && !failed; j++){
            ltfds[j] = open(ltFiles[j], O_RDONLY);

            if(ltfds[j] < 0){
                printf("cshell: no such file or directory\n");

                for(int k = 0; k < j; k++) 
                    close(ltfds[k]);

                failed = 1;
            }
        }

        int* gtfds = (int*)malloc((gtCount > 0 ? gtCount : 1) * sizeof(int));
        for(int j = 0; j < gtCount && !failed; j++){
            gtfds[j] = open(gtFiles[j], O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if(gtfds[j] < 0){

                printf("cshell: unable to create file for writing\n");
                for(int k = 0; k < j; k++)
                    close(gtfds[k]);

                failed = 1;
            }
        }

        int* gtgtfds = (int*)malloc((gtgtCount > 0 ? gtgtCount : 1) * sizeof(int));
        for(int j = 0; j < gtgtCount && !failed; j++){
            gtgtfds[j] = open(gtgtFiles[j], O_WRONLY | O_CREAT | O_APPEND, 0644);

            if(gtgtfds[j] < 0){
                printf("cshell: unable to create file for writing\n");

                for(int k = 0; k < j; k++)
                    close(gtgtfds[k]);

                failed = 1;
            }
        }


        if(failed){
            retStatus = 1;
            free(ltfds); free(gtfds); free(gtgtfds); free(resolvedPath);
            free(cmdArgv); free(ltFiles); free(gtFiles); free(gtgtFiles);
            pids[i] = -1;
            continue;
        }

        int hasInput = ltCount > 0;
        int hasOutput = (gtCount + gtgtCount) > 0;
        int hasPrevPipe = (i > 0);
        int hasNextPipe = (i < numStages - 1);

        int inPipe[2];
        int outPipe[2];
        pid_t inputPid = -1, outputPid = -1;


        // Redirection of all the input files to a single buffer
        if(hasInput){
            pipe(inPipe);

            inputPid = fork();
            if(inputPid == 0){
                close(inPipe[0]);
                closeAllStagePipes(pipeFds, numStages - 1);

                for(int j = 0; j < ltCount; j++){
                    char buf[4096];
                    int r;
                    while((r = read(ltfds[j], buf, sizeof(buf))) > 0){
                        write(inPipe[1], buf, (size_t)r);
                    }
                    close(ltfds[j]);
                }

                close(inPipe[1]);
                exit(0);
            }

            helperPids[helperCount++] = inputPid;

            for(int j = 0; j < ltCount; j++)
                close(ltfds[j]);

            close(inPipe[1]);
        }

        // Redirection of all the output files to a single buffer
        if(hasOutput){
            pipe(outPipe);

            outputPid = fork();
            if(outputPid == 0){
                close(outPipe[1]);
                closeAllStagePipes(pipeFds, numStages - 1);

                char buf[4096];
                int r;
                while((r = read(outPipe[0], buf, sizeof(buf))) > 0){
                    for(int j = 0; j < gtCount; j++){
                        write(gtfds[j], buf, (size_t)r);
                    }
                    for(int j = 0; j < gtgtCount; j++){
                        write(gtgtfds[j], buf, (size_t)r);
                    }
                }
                close(outPipe[0]);

                for(int j = 0; j < gtCount; j++)
                    close(gtfds[j]);

                for(int j = 0; j < gtgtCount; j++)
                    close(gtgtfds[j]);

                exit(0);
            }

            helperPids[helperCount++] = outputPid;

            for(int j = 0; j < gtCount; j++)
                close(gtfds[j]);

            for(int j = 0; j < gtgtCount; j++)
                close(gtgtfds[j]);

            close(outPipe[0]);
        }


        // Forking
        pid_t pid = fork();
        if(pid == 0){
            child(resolvedPath, cmdName, cmdArgv, argCount, hasInput, inPipe[0], hasOutput, outPipe[1], hasPrevPipe, hasPrevPipe ? pipeFds[i - 1][0] : -1, hasNextPipe, hasNextPipe ? pipeFds[i][1] : -1, pipeFds, numStages - 1, pgid);
        }

        if(pgid==0){
            pgid = pid;
        }
        setpgid(pid, pgid);

        if(!background){
            tcsetpgrp(STDIN_FILENO, pgid);
        }


        pids[i] = pid;

        if(hasInput) close(inPipe[0]);
        if(hasOutput) close(outPipe[1]);

        free(ltfds); free(gtfds); free(gtgtfds); free(resolvedPath);
        free(cmdArgv); free(ltFiles); free(gtFiles); free(gtgtFiles);
    }

    
    if(numStages > 1)
        closeAllStagePipes(pipeFds, numStages - 1);

    grPro[grProCount].pgid = pgid;
    grPro[grProCount].job = background ? backgroundTasks++ : 0;
    grPro[grProCount].numProcs = numStages;
    grPro[grProCount].procs = malloc(numStages * sizeof(proc));
    grPro[grProCount].background = background;

    for(int j=0;j<numStages;j++){
        grPro[grProCount].procs[j].pid = pids[j];

        char* token = stages[j]->token;
        if(token[0] == '%'){
            token++;
        }

        grPro[grProCount].procs[j].cmdName = strdup(token);
        if(pids[j] == -1){
            grPro[grProCount].procs[j].state = "Completed";
        } else {
            grPro[grProCount].procs[j].state = "Running";
        }
    }
    
    grPro[grProCount].fullCommand = strdup(fullCmdStr);
    
    int currentProcIdx = grProCount;
    grProCount++;

    if (background){
        printf("[%d] %d\n", grPro[currentProcIdx].job, (int)pgid);
    }
    else{
        tcsetpgrp(STDIN_FILENO, pgid);

        int active = 1;
        while(active) {
            active = 0;
            for (int i = 0; i < grPro[currentProcIdx].numProcs; i++) {
                if (!strcmp(grPro[currentProcIdx].procs[i].state, "Running")) {
                    active = 1;
                    break;
                }
            }
            if (active) {
                pause();
            }
        }

        tcsetpgrp(STDIN_FILENO, getpgrp());
    }

    free(fullCmdStr);

    for(int i = 0; i < helperCount; i++)
        while(waitpid(helperPids[i], NULL, 0) == -1 && errno == EINTR);

    free(pids); free(helperPids); free(stages);

    return retStatus;
}