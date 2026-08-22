#include "functions.h"

Type findType(char* token){
    if(!strcmp(token, "<"))  return LT;
    else if(!strcmp(token, ">")) return GT;
    else if(!strcmp(token, ">>")) return GTGT;
    else if(!strcmp(token, ";")) return SEMI;
    else if(!strcmp(token, "|")) return PIPE;
    else if(!strcmp(token, "&")) return AMP;
    else return WORD;

    return WORD;
}

Type assignNext(Type curr){
    if(curr == LT || curr == GT || curr == GTGT) return TGT;
    else if(curr == SEMI || curr == PIPE) return CMD;
    else if(curr == AMP) return BG;
    else if(curr == WORD) return ARG;

    return ARG;
}

int lexer(Node** node, char* word, int len, Type* nextTokenType){

    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->next = NULL;

    newNode->token = (char*)malloc((len + 1)  * sizeof(char));
    strncpy(newNode->token, word, len + 1);

    Type currType = findType(word);
    if( (*nextTokenType) == DONE || ((*nextTokenType) != ARG && currType != WORD )){
        free(newNode->token);
        free(newNode);
        return -1;
    }
    
    newNode->type = currType;
    (*nextTokenType) = assignNext(currType);

    (*node)->next = newNode;
    (*node) = newNode;
    

    return 0;
}

void freeNodes(Node* head){
    Node* temp = head;

    while(head != NULL){
        head = head->next;
        if(temp->token != NULL){
            free(temp->token);
        }
        free(temp);
        temp = head;
    }

    return;
}