#include "functions.h"

Type findType(char* token){
    if(!strcmp(token, ">"))  return LT;
    else if(!strcmp(token, "<")) return GT;
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

Node* lexer(char** tokens, int len){
    Type next;

    Node* head = (Node*)malloc(sizeof(Node));
    Node* temp = head;

    head->type = findType(tokens[0]);
    if(head->type != WORD){
        return NULL;
    }

    next = assignNext(head->type);
    
    head->next = NULL;

    for(int i=1;i<len;i++){
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->next = NULL;

        Type curr = findType(tokens[i]);
        if(next != ARG && curr != WORD) return NULL;

        next = assignNext(curr);
        newNode->type = curr;

        temp->next = newNode;
        temp = newNode;
    }

    return head;
}