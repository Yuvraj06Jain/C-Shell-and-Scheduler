#include "functions.h"

void exitShell(){
    printf("EXITING FROM THE SHELL.");
    exit(0);
}

void getPrompt(char** res){
    struct passwd* userDetails = getpwuid(getuid());
    
    char* username = userDetails->pw_name;

    char hostname[500];
    if(gethostname(hostname, sizeof(hostname))){
       printf("ERROR : Could not Fetch hostname.\n");
       exitShell();
    }

    char buffer[1000];
    char* cwd = getcwd(buffer, sizeof(buffer));
    if(cwd == NULL){
        printf("ERROR : Could not fetch the current working directory.\n");
        exitShell();
    }

    static char* homeDir = NULL;
    if(homeDir == NULL){
        homeDir = getcwd(NULL, 0);
        if(homeDir == NULL){
            printf("ERROR : Could not fetch current home directory.\n");
            exitShell();
        }
    }

    char* displayPath = NULL;
    
    if(!strcmp(homeDir, cwd)){
        displayPath = "~";
    }
    else if( (!strncmp(cwd, homeDir, strlen(homeDir))) && cwd[strlen(homeDir)] == '/'){
        displayPath = cwd + strlen(homeDir);
    }
    else{
        displayPath = cwd;
    }

    char shrinkedCwd[1000];

    if(displayPath[0] == '/'){
        snprintf(shrinkedCwd, strlen(displayPath), "~%s", displayPath);
    }
    else{
        snprintf(shrinkedCwd, strlen(displayPath), "%s", displayPath);
    }


    int prompt_length = strlen(username) + strlen(hostname) + strlen(shrinkedCwd) + 10;
    char* prompt = (char*)malloc(prompt_length);

    sprintf(prompt, "<%s@%s:%s>", username, hostname, shrinkedCwd);
    
    *res = prompt;

    return;
}

void prompt(){
    char* propt = NULL;
    getPrompt(&propt);
    printf("%s ", propt);
    free(propt);
}
