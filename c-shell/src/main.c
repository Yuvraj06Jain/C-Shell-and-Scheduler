#include "functions.h"

int no_commands = 1;

int main(){
    printf("\n================================================================================================================================\n\n");

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
            break;
        }
        else if(!strcmp(input, "\n\0")){
            continue;
        }
        
        int error = 0;
        Node* llHead = parse(input, &error);

        if(error == 1){
            printf("c-shell : invalid syntax.\n");
            continue;
        }

        Node* temp = llHead;
        while(temp!=NULL){
            printf("TYPE = %d | String = %s\n", temp->type, temp->token);
            temp = temp->next;
        }

        // Node* temp = llHead; int idx = 0;

        // for(int i=0;i<no_commands;i++){
        //     if( temp->type == WORD && strcmp(tokens[idx], cmds[i]) ){
        //         hop(temp->next);
        //     }
        // }

        freeNodes(llHead);
        free(input);
    }
    printf("\n================================================================================================================================\n\n");
    return 0;
}