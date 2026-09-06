#include "functions.h"
#include "hop.h"

char* homeDir = NULL;
char* cwd = NULL;
char* username = NULL;
char hostname[500];     
hisNode* prevHead = NULL;
hopNode* hopHead = NULL;
hopNode* hopTail = NULL;
int no_commands = 4;


void exitShell(){
    printf("EXITING FROM THE SHELL.");
    exit(0);
}


int main(){
    printf("\n================================================================================================================================\n\n");

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

        Node* temp = llHead;
        while(temp!=NULL){
            printf("Token Type: %d, Token: %s\n", temp->type, temp->token);
            temp = temp->next;
        }

        // Execution
        execCmds(llHead);

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
    Node** cmds = (Node**)malloc(numCmds * sizeof(Node*)); int cmdIdx = 0;

    Node* temp = llHead;

    while(temp!=NULL){
        if (temp->type == SEMI){
            if (numCmds == cmdIdx){
                numCmds = numCmds * 2;
                cmds = (Node**)realloc(cmds, numCmds * sizeof(Node*));
            }

            cmds[cmdIdx++] = temp;
        }
        temp = temp->next;
    }

    if (numCmds == cmdIdx){
        numCmds = numCmds * 2;
        cmds = (Node**)realloc(cmds, numCmds * sizeof(Node*));
    }
    cmds[cmdIdx++] = NULL;
    numCmds = cmdIdx;

    temp = llHead;
    for(int i=0;i<numCmds;i++){
        
        int ret = 0;
        if(!strcmp(temp->token, "hop")){
            ret = hop(temp->next, cmds[i]);
        }
        else{
            ret = execute(temp, cmds[i]);
        }

        if (ret!=0){
            break;
        }

        // Advance temp to the start of the next command (skip past the SEMI node)
        temp = cmds[i];
        if(temp != NULL)
            temp = temp->next;
    }

    free(cmds);
    return;
}