#include "functions.h"

int main(){
    printf("\n================================================================================================================================\n\n");

    while(1){
        prompt();
        
        char* input = NULL;
        size_t bufsize = 0;

        getline(&input, &bufsize, stdin);
        if(!strcmp(input, "EXIT\n")){
            break;
        }
        else if(!strcmp(input, "\n\0")){
            continue;
        }
        
        char** tokens = NULL;
        int len = parse(input, &tokens);

        if(len == -1){
            printf("c-shell : invalid syntax\n");
            continue;
        }

        // for(int i=0;i<len;i++){
        //     printf("%s\n", tokens[i]);
        // }

        Node* llHead = lexer(tokens, len);
        if(llHead == NULL){
            printf("c-shell : invalid syntax\n");
            continue;
        }

        Node* temp = llHead;
        int idx = 0;
        while(temp!=NULL){
            printf("TYPE = %d | String = %s\n", temp->type, tokens[idx++]);
            temp = temp->next;
        }
    }

    printf("\n================================================================================================================================\n\n");
    return 0;
}