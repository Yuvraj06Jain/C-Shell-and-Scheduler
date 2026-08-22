#include "functions.h"

char space_characters[] = {' ', '\n', '\r', '\t'};
char special_characters[] = {'|', '&', '>', '<', ';'};

bool in(char c, char* arr, int len){
    for(int i=0;i<len;i++){
        if(c == arr[i]){
            return true;
        }
    }
    return false;
}

Node* parse(char* line, int* error){

    Node* buffer = (Node*)malloc(sizeof(Node));
    buffer->next = NULL; buffer->token = NULL; buffer->type = WORD;

    Node* temp = buffer;

    Type nextTokenType = WORD;

    bool inside_dq = false; bool inside_sq = false;

    bool escaped = false;

    int n = strlen(line);

    int word_size = 1024; int idx = 0;
    char* word = (char*)malloc(word_size * sizeof(char));

    for(int i=0;i<n;i++){

        // Outside Quotes
        if(!inside_dq && !inside_sq){

            // Escaped Processing
            if(escaped){
                word[idx++] = line[i];
                escaped = false;
                continue;
            }

            // Checking for space
            if(in(line[i], space_characters, 4)){
                
                if(idx == 0){
                    continue;
                }

                word[idx] = '\0';
                int ret = lexer(&temp, word, idx, &nextTokenType);

                if(ret == -1){
                    free(word);
                    freeNodes(buffer);
                    *error = 1;
                    return NULL;
                }

                idx = 0;
                continue;
            }

            // Checking for Special Characters
            if(in(line[i], special_characters, 5)){

                if(idx > 0){
                    word[idx] = '\0';
                    int ret = lexer(&temp, word, idx, &nextTokenType);

                    if(ret == -1){
                        free(word);
                        freeNodes(buffer);
                        *error = 1;
                        return NULL;
                    }

                    idx = 0;
                }

                word[idx++] = line[i];
                if(line[i] == '>' && i != n-1 && line[i+1] == '>'){
                    word[idx++] = '>';
                    i++;
                }

                word[idx] = '\0';
                int ret = lexer(&temp, word, idx, &nextTokenType);

                if(ret == -1){
                    free(word);
                    freeNodes(buffer);
                    *error = 1;
                    return NULL;
                }

                idx = 0;
                continue;
            }


            if(line[i] == '\\'){
                // End character as /
                if(i == n-1){
                    free(word);
                    freeNodes(buffer);
                    *error = 1;
                    return NULL;
                }
                
                escaped = true;
                continue;
            }

            if(line[i] == '\''){
                inside_sq = true;
                continue;
            }
            else if(line[i] == '\"'){
                inside_dq = true;
                continue;
            }

            // Just a Normal Character
            word[idx++] = line[i];
        }

        // Inside Double Quotes
        if(inside_dq){
            if(!escaped && line[i] == '\\'){
                escaped = true;
                continue;
            }

            if(escaped){
                // checking if " or \ are escaped
                if(line[i] == '\"' || line[i] == '\\'){
                    word[idx++] = line[i];
                    continue;
                }

                // ANY_CHAR after \ treated as different characters
                word[idx++] = '\\';
                word[idx++] = line[i];

                escaped = false;
                continue;
            }

            if(line[i] == '\"'){
                inside_dq = false;
                continue;
            }

            // Just a normal character inside a dq
            word[idx++] = line[i];
        }
        if(inside_sq){
            
            if(line[i] == '\''){
                inside_sq = false;
                continue; 
            }

            word[idx++] = line[i];
            continue;
        }
    }

    if(idx > 0){
        word[idx] = '\0';
        int ret = lexer(&temp, word, idx, &nextTokenType);

        if(ret == -1){
            free(word);
            freeNodes(buffer);
            *error = 1;
            return NULL;
        }
        idx = 0;
    }

    if(inside_dq || inside_sq || escaped || nextTokenType == CMD || nextTokenType == TGT){
        free(word);
        freeNodes(buffer);
        *error = 1;
        return NULL;
    }

    free(word);
    return buffer->next;
}