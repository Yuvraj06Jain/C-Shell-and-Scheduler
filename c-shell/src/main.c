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

        if(!strcmp(llHead->token, "hop")){
            hop(llHead->next);
        }
        else{
            execute(llHead);
        }

        freeNodes(llHead);
        free(input);
    }

    printf("\n\nDumping the Records...\n");
    dumpHopList(hopHead);

    printf("\n================================================================================================================================\n\n");

    return 0;
}

