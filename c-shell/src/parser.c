#include "functions.h"

char space_characters[] = {' ', '\n', '\r'};
char special_characters[] = {'|', '&', '>', '<', ';'};

bool in(char c, char* arr, int len){
    for(int i=0;i<len;i++){
        if(c == arr[i]){
            return true;
        }
    }
    return false;
}

int parse(char* line, char*** tokens){
    int len = 50; int idx = 0;
    (*tokens) = (char**)malloc(len * sizeof(char*));

    bool inside_dq = false; bool inside_sq = false; bool escaped = false;

    int i=0; int j=0;
    int n = strlen(line);
    while(i<n){

        if(!inside_dq && !inside_sq && in(line[i], space_characters, 3)){

            if(i>j){
                (*tokens)[idx] = (char*)malloc((i - j + 1) * sizeof(char));
                strncpy((*tokens)[idx], line + j, i - j);
                (*tokens)[idx++][i - j] = '\0';
            }

            i++; j = i;
        }
        else if(!inside_dq && !inside_sq && in(line[i], special_characters, 5)){

            if(i>j){
                (*tokens)[idx] = (char*)malloc((i - j + 1) * sizeof(char));
                strncpy((*tokens)[idx], line + j, i - j);
                (*tokens)[idx++][i - j] = '\0';
            }

            if(line[i]=='>' && i<n-1 && line[i+1] == '>'){
                (*tokens)[idx] = (char*)malloc(3 * sizeof(char));
                strncpy((*tokens)[idx], line + i, 2);
                (*tokens)[idx++][2] = '\0';

                i += 2;
            }
            else{
                (*tokens)[idx] = (char*)malloc(2 * sizeof(char));
                strncpy((*tokens)[idx], line + i, 1);
                (*tokens)[idx++][1] = '\0';

                i++;
            }

            j = i;
        }
        else{

            if(line[i] == '\'' && !escaped){
                inside_sq = !inside_sq;
            }
            if(line[i] == '\"' && !escaped){
                inside_dq = !inside_dq;
            }

            if(!inside_sq && !inside_dq && line[i] == '\\'){
                escaped = true;
            }

            i++;
        }
    }

    if(i>j){
        (*tokens)[idx] = (char*)malloc((i - j + 1) * sizeof(char));
        strncpy((*tokens)[idx], line + j, i - j);
        (*tokens)[idx++][i - j] = '\0';
    }

    if(inside_dq || inside_sq || escaped){
        return -1;
    }

    return idx;
}