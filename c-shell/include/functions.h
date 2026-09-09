#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

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

typedef struct cmdNode{
    struct Node* node;
    bool background;
}cmdNode;

typedef struct proc{
    pid_t pid;
    char* cmdName;
    char* state;
}proc;


typedef struct grpProcess{
    int job;
    pid_t pgid;
    proc* procs;
    int numProcs;
    char* fullCommand;
    bool background;
}grpProcess;

// Constants
extern char* homeDir;
extern char* cwd;
extern char hostname[500];
extern char* username;

extern hisNode* prevHead;

extern int backgroundTasks;
extern grpProcess grPro[512];
extern int grProCount;

// Functions
void exitShell();

void getPrompt(char** res);
void prompt();

Node* parse(char* line, int* error);

int lexer(Node** node, char* word, int len, Type* nextTokenType);
void freeNodes(Node* head);

int getPathDirs(char*** pathDirs);

int hop(Node* args, Node* end);

int execute(Node* args, Node* end, int background);

int execCmds(Node* llHead);

int activities();
int resume(Node* args, Node* end);
int ping(Node* args, Node* end);
int spy(Node* args, Node* end);