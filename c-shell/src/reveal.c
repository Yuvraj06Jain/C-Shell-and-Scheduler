#include "functions.h"

int hidden = 0; int recursive = 0;

int lexcmp(const void* a, const void* b){
    const char *sa = *(const char**)a;
    const char *sb = *(const char**)b;
    return strcmp(sa, sb);
}

void listDir(char* path, char* prefix){
    DIR* dir = opendir(path);
    if(dir == NULL) return;

    char** dirNames = NULL;
    int count = 0, cap = 0;

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL){
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        if(entry->d_name[0] == '.' && !hidden) continue;

        if(count == cap){
            cap = cap ? cap * 2 : 8;
            dirNames = (char**)realloc(dirNames, cap * sizeof(char*));
        }
        dirNames[count] = (char*)malloc((strlen(entry->d_name) + 1) * sizeof(char));
        strcpy(dirNames[count], entry->d_name);
        count++;
    }
    closedir(dir);

    qsort(dirNames, count, sizeof(char*), lexcmp);

    for(int i = 0; i < count; i++){
        char fullPath[4096];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, dirNames[i]);

        struct stat st;
        int isDir = (lstat(fullPath, &st) == 0 && S_ISDIR(st.st_mode));

        printf("%s%s%s\n", prefix, dirNames[i], (isDir && recursive) ? "/" : "");

        if(isDir && recursive){
            char temp[4096];
            snprintf(temp, sizeof(temp), "%s%s/", prefix, dirNames[i]);
            listDir(fullPath, temp);
        }

        free(dirNames[i]);
    }
    free(dirNames);
}

int main(int argc, char* argv[]){
    hidden = 0; recursive = 0;


    char* pathArg = NULL;
    int pathArgCount = 0;

    for(int i=1;i<argc;i++){
        char* token = argv[i];

        if(token[0] == '-' && (int)strlen(token) > 1 && token[1] != '/'){
            if(pathArgCount > 0){
                printf("reveal: invalid syntax\n");
                return 1;
            }
            for(int j = 1; token[j] != '\0'; j++){
                if(token[j] == 'a') hidden = 1;
                else if(token[j] == 't') recursive = 1;
                else{
                    printf("reveal: invalid syntax\n");
                    return 1;
                }
            }
        }
        else{
            pathArgCount++;
            if(pathArgCount > 1){
                printf("reveal: invalid syntax\n");
                return 1;
            }
            pathArg = token;
        }

    }

    char* target = NULL;
    int freeTarget = 0;

    if(pathArg == NULL || !strcmp(pathArg, ".")){
        target = getcwd(NULL, 0);
    }
    else if(!strcmp(pathArg, "~")){
        target = getenv("CSHELL_HOME");
    }
    else if(!strcmp(pathArg, "-")){
        if(getenv("OLDCWD") == NULL){
            printf("reveal: no such directory\n");
            return 1;
        }
        target = getenv("OLDCWD");
    }
    else{
        target = realpath(pathArg, NULL);
        if(target == NULL){
            printf("reveal: no such directory\n");
            return 1;
        }
        freeTarget = 1;
    }

    struct stat st;
    if(stat(target, &st) != 0 || !S_ISDIR(st.st_mode)){
        printf("reveal: no such directory\n");
        if(freeTarget) free(target);
        return 1;
    }

    listDir(target, "");

    if(freeTarget) free(target);
    return 0;
}