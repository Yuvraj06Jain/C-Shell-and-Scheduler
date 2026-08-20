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
    BG,
    DONE
}Type;

typedef struct Node{
    Type type;
    char* token;
    struct Node* next;
}Node;


typedef int (*cmd_func)(Node* args);

typedef struct cmd{
    char* cmd_name;
    cmd_func func;
}cmd;

cmd cmds[] = {
    {"hop", hop}
};


void exitShell();

void getPrompt(char** res);
void prompt();

Node* parse(char* line, int* error);

int lexer(Node** node, char* word, int len, Type* nextTokenType);
void freeNodes(Node* head);

int hop(Node* tokens);