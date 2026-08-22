#include "functions.h"

void getPrompt(char** res){

    char* displayPath = NULL;
    bool underHome = false;
    
    if(!strcmp(homeDir, cwd)){
        displayPath = "~";
    }
    else if( (!strncmp(cwd, homeDir, strlen(homeDir))) && cwd[strlen(homeDir)] == '/'){
        displayPath = cwd + strlen(homeDir);
        underHome = true;
    }
    else{
        displayPath = cwd;
    }

    char shrinkedCwd[1024];

    if(underHome){
        snprintf(shrinkedCwd, 1024, "~%s", displayPath);
    }
    else{
        snprintf(shrinkedCwd, 1024, "%s", displayPath);
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
