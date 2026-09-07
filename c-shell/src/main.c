#include "functions.h"
#include "hop.h"

char* homeDir = NULL;
char* cwd = NULL;
char* username = NULL;
char hostname[500];

hisNode* prevHead = NULL;
hopNode* hopHead = NULL;
hopNode* hopTail = NULL;

int backgroundTasks = 1;
pid_t bg_pids[512];
int bg_count = 0;

pid_t finished_pids[512];
int finished_status[512];
int finished_count = 0;

void exitShell(){
    printf("EXITING FROM THE SHELL.");
    exit(0);
}


void sigchild_handler(int sig){
    int status;
    pid_t pid;
    
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        for(int i = 0; i < bg_count; i++){
            if(bg_pids[i] == pid){
                finished_pids[finished_count] = pid;
                finished_status[finished_count] = status;
                finished_count++;
                bg_pids[i] = bg_pids[--bg_count];
                break;
            }
        }
    }
}

void printBg(){
    for(int i = 0; i < finished_count; i++){
        if(WIFEXITED(finished_status[i])){
            char buf[256];
            int len = snprintf(buf, sizeof(buf), "\ncommand with pid %d exited normally.\n", finished_pids[i]);
            write(STDOUT_FILENO, buf, len);
        } else {
            char buf[256];
            int len = snprintf(buf, sizeof(buf), "\ncommand with pid %d exited normally.\n", finished_pids[i]);
            write(STDOUT_FILENO, buf, len);
        }
    }
    finished_count = 0;
}

int main(){
    printf("\n================================================================================================================================\n\n");

    // Setting up the signal handler

    struct sigaction sa;
    sa.sa_handler = sigchild_handler;
    sa.sa_flags = SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);



    // Setting up the Home Directory.
    homeDir = getcwd(NULL, 0);
    if(homeDir == NULL){
        printf("ERROR : Could not fetch current home directory.\n");
        exitShell();
    }

    // Setting up the home as env variable
    setenv("CSHELL_HOME", homeDir, 1);

    // Setting up the current Working Directory
    cwd = strdup(homeDir);

    // Setting up the username and the hostname
    struct passwd* userDetails = getpwuid(getuid());

    username = userDetails->pw_name;

    if(gethostname(hostname, sizeof(hostname))){
       printf("ERROR : Could not Fetch hostname.\n");
       exitShell();
    }

    // Setting up the Hop Records
    printf("Caching the Hop Records...\n");
    freqPair p = createHopList();
    hopHead = p.first; hopTail = p.second;    

    while(1){
        prompt();
        
        char* input = NULL;
        size_t bufsize = 0;

        // Input Hanlding
        int ret = getline(&input, &bufsize, stdin);
        if(ret == -1){
            if(errno == EINTR){
                clearerr(stdin);
                printBg();
                free(input);
                continue;
            }
            free(input);
            break;
        }
        if(!strcmp(input, "EXIT\n")){
            free(input);
            break;
        }
        else if(!strcmp(input, "\n\0")){
            free(input);
            continue;
        }
        
        // Parsing and Error Hanlding
        int error = 0;
        Node* llHead = parse(input, &error);

        if(error == 1){
            printf("c-shell : invalid syntax.\n");
            free(input);
            continue;
        }
        if(llHead == NULL){
            free(input);
            continue;
        }

        // Node* temp = llHead;
        // while(temp!=NULL){
        //     printf("Token Type: %d, Token: %s\n", temp->type, temp->token);
        //     temp = temp->next;
        // }

        // Execution
        execCmds(llHead);
        printBg();

        freeNodes(llHead);
        free(input);
    }

    printf("\n\nDumping the Records...\n");
    dumpHopList(hopHead);

    printf("\n================================================================================================================================\n\n");

    return 0;
}

void execCmds(Node* llHead){
    int numCmds = 1;
    cmdNode** cmds = (cmdNode**)malloc(numCmds * sizeof(cmdNode*)); int cmdIdx = 0;

    Node* temp = llHead;

    while(temp!=NULL){
        if (temp->type == SEMI){
            if (numCmds == cmdIdx){
                numCmds = numCmds * 2;
                cmds = (cmdNode**)realloc(cmds, numCmds * sizeof(cmdNode*));
            }

            cmds[cmdIdx] = (cmdNode*)malloc(sizeof(cmdNode));
            cmds[cmdIdx]->node = temp;
            cmds[cmdIdx++]->background = false;
        }
        else if(temp->type == AMP){
            if (numCmds == cmdIdx){
                numCmds = numCmds * 2;
                cmds = (cmdNode**)realloc(cmds, numCmds * sizeof(cmdNode*));
            }

            cmds[cmdIdx] = (cmdNode*)malloc(sizeof(cmdNode));
            cmds[cmdIdx]->node = temp;
            cmds[cmdIdx++]->background = true;
        }

        temp = temp->next;
    }

    if (numCmds == cmdIdx){
        numCmds = numCmds * 2;
        cmds = (cmdNode**)realloc(cmds, numCmds * sizeof(cmdNode*));
    }

    cmds[cmdIdx] = (cmdNode*)malloc(sizeof(cmdNode));
    cmds[cmdIdx]->node = temp;
    cmds[cmdIdx++]->background = false;
    numCmds = cmdIdx;

    temp = llHead;
    for(int i=0;i<numCmds;i++){
        if(temp == NULL){
            break;
        }
        
        int ret = 0;
        if(!strcmp(temp->token, "hop")){
            ret = hop(temp->next, cmds[i]->node);
        }
        else{
            ret = execute(temp, cmds[i]->node, cmds[i]->background);
        }

        if (ret!=0){
            break;
        }

        temp = cmds[i]->node;
        if(temp != NULL)
            temp = temp->next;
    }

    for(int i=0;i<numCmds;i++){
        free(cmds[i]);
    }
    free(cmds);
    return;
}