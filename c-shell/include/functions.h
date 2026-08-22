#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>


// Structs and Enums
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

typedef struct hisNode{
    char* dirName;
    struct hisNode* prev;
}hisNode;


// Constants

extern char* homeDir;
extern char* cwd;
extern char hostname[500];
extern char* username;

extern hisNode* prevHead;

// Functions
void exitShell();

void getPrompt(char** res);
void prompt();

Node* parse(char* line, int* error);

int lexer(Node** node, char* word, int len, Type* nextTokenType);
void freeNodes(Node* head);

void pushHis(hisNode** hisHead, char* dirName);
char* popHis(hisNode** hisHead);
void freeHis(hisNode* hisHead);

int hop(Node* args);
