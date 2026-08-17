#include "include/functions.h"

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

    char* homeDir = userDetails->pw_dir;

    if(strlen(cwd) < strlen(homeDir)){
        int prompt_length = strlen(username) + strlen(hostname) + strlen(cwd) + 10;
        char prompt[prompt_length];

        sprintf(prompt, "<%s@%s:%s>:", username, hostname, cwd);
        *res = prompt;

        return;
    }

    char temp[strlen(cwd) + 1];
    strncpy(temp, cwd + strlen(homeDir), strlen(cwd) - strlen(homeDir) + 1);
    
    char shrinkedCwd[strlen(cwd) + 10];
    if(strlen(temp)==0){
        sprintf(shrinkedCwd, "~");
    }
    else{
        sprintf(shrinkedCwd, "~/%s", temp);
    }

    int prompt_length = strlen(username) + strlen(hostname) + strlen(shrinkedCwd) + 10;
    char prompt[prompt_length];

    sprintf(prompt, "<%s@%s:%s>:", username, hostname, shrinkedCwd);
    *res = prompt;

    return;
}

void prompt(){
    char* propt = NULL;
    getPrompt(&propt);
    printf("%s\n", propt);
}

int main(){
    prompt();

    return 0;
}


