#include "functions.h"
#include "hop.h"

char hopFilePath[] = "/home/yuvraj/Desktop/Projects/C-Shell-and-Scheduler/c-shell/include/hopFile.txt";

freqPair createHopList(){
    FILE* hopFile = fopen(hopFilePath, "r");
    if(hopFile == NULL){
        printf("ERROR : Couldn't create the Hop Frequency List.\n\n");
        freqPair empty = {NULL, NULL};
        return empty;
    }

    hopNode* buffer = (hopNode*)malloc(sizeof(hopNode));
    buffer->freq = 0; buffer->dirList = NULL; buffer->next = NULL; buffer->prev = NULL;
    hopNode* temp = buffer;

    char line[1000];
    while(fgets(line, sizeof(line), hopFile)){

        char* endPtr = NULL;
        long freq = strtol(line, &endPtr, 10);
        if(endPtr == line){
            continue;
        }

        char* rest = endPtr;
        if(*rest == '\t')
            rest++;


        dirNode* buff = (dirNode*)malloc(sizeof(dirNode));
        buff->dirName = NULL; buff->next = NULL; buff->prev = NULL;
        dirNode* tmp = buff;

        char dirName[1024]; int idx = 0;
        for(int i=0;i<(int)strlen(rest);i++){
            if(line[i] == '\t'){
                if(idx == 0){
                    continue;
                }

                dirName[idx] = '\0';
                dirNode* newNode = (dirNode*)malloc(sizeof(dirNode));
                newNode->next = NULL; newNode->prev = NULL;

                newNode->dirName = (char*)malloc((strlen(dirName) + 1) * sizeof(char));
                strncpy(newNode->dirName, dirName, strlen(dirName) + 1);

                newNode->prev = tmp;
                tmp->next = newNode;
                tmp = newNode;

                idx = 0;
            }
            else{
                dirName[idx++] = line[i];
            }
        }

        if(buff->next == NULL){
            free(buff);
            continue;
        }

        hopNode* newFreq = (hopNode*)malloc(sizeof(hopNode));
        newFreq->freq = (int)freq; newFreq->next = NULL; newFreq->prev = NULL;
        newFreq->dirList = buff->next;

        if(newFreq->dirList != NULL){
            newFreq->dirList->prev = NULL;
        }

        newFreq->prev = temp;
        temp->next = newFreq;
        temp = newFreq;

        free(buff);
    }

    fclose(hopFile);

    freqPair fp;
    fp.first = buffer->next;
    fp.second = (!buffer->next) ? NULL : temp;

    if(fp.first != NULL){
        fp.first->prev = NULL;
    }
    free(buffer);

    printf("Caching Successfull.\n\n");
    return fp;
}

void dumpHopList(hopNode* freqHead){
    FILE* hopFile = fopen(hopFilePath, "w");
    if(hopFile == NULL){
        printf("ERROR : Failed to Update the Hop Frequency Records\n");
        return;
    }
    
    hopNode* temp = freqHead;
    
    while(temp!=NULL){
        fprintf(hopFile, "%d\t", temp->freq);

        dirNode* tmp = temp->dirList;

        while(tmp != NULL){
            fprintf(hopFile, "%s\t\t", tmp->dirName);

            dirNode* tmp1 = tmp->next;
            free(tmp->dirName);
            free(tmp);
            tmp = tmp1;
        }
        fprintf(hopFile, "\n");

        hopNode* tempNext = temp->next;
        free(temp);
        temp = tempNext;
    }

    fclose(hopFile);

    return;
}

pair findRecord(hopNode* freqHead, char* dirName){
    hopNode* temp = freqHead;

    while(temp!=NULL){
        
        hopNode* tempNext = temp->next;
        dirNode* tmp = temp->dirList;

        while(tmp!=NULL){
            dirNode* tmpNext = tmp->next;

            if(access(tmp->dirName, F_OK) != 0){
                pair p = {temp, tmp};
                deleteNode(&p, &hopHead, &hopTail);
            }
            else if(!strcmp(tmp->dirName, dirName)){
                pair p = {temp, tmp};
                return p;
            }

            tmp = tmpNext;
        }
        temp = tempNext;
    }

    pair p = {NULL, NULL};
    return p;
}


void pushRecord(hopNode** freqHead, hopNode** freqTail, char* dirName){
    pair p = findRecord(*freqHead, dirName);
    hopNode* oldFreqNode = p.first;
    hopNode* freqNode;

    if(*freqHead == NULL){
        hopNode* newFreq = (hopNode*)malloc(sizeof(hopNode));
        newFreq->freq = 1; newFreq->prev = NULL; newFreq->next = NULL; newFreq->dirList = NULL;
        *freqHead = newFreq;
        *freqTail = newFreq;
        freqNode = newFreq;
    }
    else if(oldFreqNode == NULL){
        freqNode = *freqHead;
    }
    else if(oldFreqNode->next == NULL || oldFreqNode->next->freq != oldFreqNode->freq + 1){

        hopNode* newFreq = (hopNode*)malloc(sizeof(hopNode));
        newFreq->freq = oldFreqNode->freq + 1; newFreq->dirList = NULL; newFreq->prev = oldFreqNode; newFreq->next = oldFreqNode->next;

        if(oldFreqNode->next != NULL){
            oldFreqNode->next->prev = newFreq;
        }
        else{
            *freqTail = newFreq;
        }

        oldFreqNode->next = newFreq;
        freqNode = newFreq;
    }
    else{
        freqNode = oldFreqNode->next;
    }

    dirNode* newNode = (dirNode*)malloc(sizeof(dirNode));
    newNode->next = NULL;
    newNode->dirName = (char*)malloc((strlen(dirName) + 1) * sizeof(char));
    strcpy(newNode->dirName, dirName);

    if(freqNode->dirList == NULL){
        newNode->prev = NULL;
        freqNode->dirList = newNode;
    }
    else{
        dirNode* temp = freqNode->dirList;
        while(temp->next != NULL){
            temp = temp->next;
        }
        newNode->prev = temp;
        temp->next = newNode;
    }

    if(oldFreqNode != NULL){
        deleteNode(&p, freqHead, freqTail);
    }

    return;
}

void deleteNode(pair* p, hopNode** freqHead, hopNode** freqTail){
    if(p->first == NULL) return;

    hopNode* freqNode = p->first;
    dirNode* dir = p->second;


    if(dir->prev == NULL){
        freqNode->dirList = dir->next;
        if(dir->next != NULL){
            dir->next->prev = NULL;
        }
    }
    else{
        dir->prev->next = dir->next;
        if(dir->next != NULL){
            dir->next->prev = dir->prev;
        }
    }

    free(dir->dirName);
    free(dir);

    if(freqNode->dirList == NULL){
        if(freqNode->prev == NULL){
            *freqHead = freqNode->next;
        } else {
            freqNode->prev->next = freqNode->next;
        }

        if(freqNode->next == NULL){
            *freqTail = freqNode->prev;
        } else {
            freqNode->next->prev = freqNode->prev;
        }

        free(freqNode);
    }

    return;
}

pair findBestMatch(hopNode* freqEnd, char* dirName){
    if(freqEnd == NULL){
        pair p = {NULL, NULL};
        return p;
    }

    hopNode* temp = freqEnd;
    while(temp != NULL){
        hopNode* tempPrev = temp->prev;
        dirNode* tmp = temp->dirList;
        dirNode* match = NULL;

        while(tmp != NULL){
            dirNode* tmpNext = tmp->next;

            if(access(tmp->dirName, F_OK) != 0){
                pair p = {temp, tmp};
                deleteNode(&p, &hopHead, &hopTail);
            }
            else if(strstr(tmp->dirName, dirName) != NULL){
                match = tmp;
            }

            tmp = tmpNext;
        }

        if(match != NULL){
            pair p = {temp, match};
            return p;
        }

        temp = tempPrev;
    }

    pair p = {NULL, NULL};
    return p;
}


int hop(Node* args){

    if(args == NULL){

        if(chdir(homeDir) != 0){
            printf("hop: no such directory\n");
            return 1;
        }

        setenv("OLDCWD", cwd, 1);
        free(cwd);
        cwd = getcwd(NULL, 0);

        pushRecord(&hopHead, &hopTail, cwd);

        return 0;
    }

    Node* argTemp = args;
    while(argTemp!=NULL){
        char* token = argTemp->token;

        // Handling the '~' case
        if(!strcmp(token, "~")){
            if(chdir(homeDir) != 0){
                printf("hop : no such directory\n");
                return 1;
            }

            setenv("OLDCWD", cwd, 1);
            free(cwd);
            cwd = getcwd(NULL, 0);

            pushRecord(&hopHead, &hopTail, cwd);
        }
        // Handling the '-' case
        else if(!strcmp(token, "-")){

            if(getenv("OLDCWD") == NULL){
                argTemp = argTemp->next;
                continue;
            }

            char* oldCwd = strdup(cwd);

            char* prevDir = getenv("OLDCWD");

            int ret = chdir(prevDir);

            if(ret != 0){
                free(oldCwd);
                printf("hop: no such directory\n");
                return 1;
            }

            setenv("OLDCWD", oldCwd, 1);
            free(oldCwd);
            free(cwd);
            cwd = getcwd(NULL, 0);

            pushRecord(&hopHead, &hopTail, cwd);
        }
        // Handling the '.' case
        else if(!strcmp(token, ".")){
            argTemp = argTemp->next;
            continue;
        }
        // Handling the ".." case
        else if(!strcmp(token, "..")){
            char* oldCwd = cwd;

            if(chdir("..") != 0){
                printf("hop: no such directory\n");
                return 1;
            }

            char* newCwd = getcwd(NULL, 0);
            if(newCwd == NULL){
                argTemp = argTemp->next;
                continue;
            }

            if(!strcmp(newCwd, oldCwd)){
                argTemp = argTemp->next;
                free(newCwd);
                continue;
            }

            setenv("OLDCWD", cwd, 1);
            free(cwd);
            cwd = newCwd;

            pushRecord(&hopHead, &hopTail, cwd);
        }
        // Handling other cases
        else{
            char* absPath = realpath(token, NULL);

            if(absPath == NULL){
                pair p = findBestMatch(hopTail, token);

                if(p.first == NULL){
                    printf("hop: no such directory\n");
                    return 1;
                }

                if(chdir(p.second->dirName) != 0){
                    printf("hop: no such directory\n");
                    return 1;
                }

                setenv("OLDCWD", cwd, 1);
                free(cwd);
                cwd = getcwd(NULL, 0);

                pushRecord(&hopHead, &hopTail, cwd);
            }
            else{
                int ret = chdir(absPath);
                free(absPath);

                if(ret != 0){
                    printf("hop: no such directory\n");
                    return 1;
                }

                setenv("OLDCWD", cwd, 1);
                free(cwd);
                cwd = getcwd(NULL, 0);

                pushRecord(&hopHead, &hopTail, cwd);
            }
        }

        argTemp = argTemp->next;
    }

    return 0;
}