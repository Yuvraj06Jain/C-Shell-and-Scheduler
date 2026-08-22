#include "functions.h"
#include "hop.h"


void pushHis(hisNode** hisHead, char* dirName){

    hisNode* newNode = (hisNode*)malloc(sizeof(hisNode));
    newNode->prev = NULL;

    newNode->dirName = (char*)malloc((strlen(dirName) + 1) * sizeof(char));
    strcpy(newNode->dirName, dirName);

    newNode->prev = (*hisHead);
    (*hisHead) = newNode;
}

char* popHis(hisNode** hisHead){
    hisNode* temp = (*hisHead);
    (*hisHead) = temp->prev;

    char* res = temp->dirName;
    free(temp);

    return res;
}

void freeHis(hisNode* hisHead){
    hisNode* temp = hisHead;

    while(temp!=NULL){
        temp = hisHead->prev;

        free(hisHead->dirName);
        free(hisHead);

        hisHead = temp;
    }

    return;
}