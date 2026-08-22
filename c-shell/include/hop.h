typedef struct dirNode{
    char* dirName;
    struct dirNode* next;
    struct dirNode* prev;
}dirNode;

typedef struct hopNode{
    int freq;
    dirNode* dirList;
    struct hopNode* next;
    struct hopNode* prev;
}hopNode;


typedef struct pair{
    hopNode* first;
    dirNode* second;
}pair;

typedef struct freqPair{
    hopNode* first;
    hopNode* second;
}freqPair;

extern hopNode* hopHead;
extern hopNode* hopTail;

freqPair createHopList();
void dumpHopList(hopNode* frqHead);
pair findRecord(hopNode* frqHead, char* dirName);
void pushRecord(hopNode** frqHead, hopNode** frqTail, char* dirName);
void deleteNode(pair* p, hopNode** frqHed, hopNode** frqTail);
pair findBestMatch(hopNode* frqEnd, char* dirName);

int hop(Node* args);