#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

typedef enum Type{
    WORD,
    ARG,
    LT,
    GT,
    GTGT,
    TGT,
    PIPE,
    SEMI,
    AMP,
    CMD,
    BG
}Type;

typedef struct Node{
    Type type;
    struct Node* next;
}Node;


void exitShell();

void getPrompt(char** res);
void prompt();

int parse(char* line, char*** tokens);

Node* lexer(char** tokens, int len);