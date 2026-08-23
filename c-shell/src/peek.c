#include "functions.h"

int numbered = 0; int reversed = 0;

int countNonEmpty(int fd){
    int chunk = 4096;

    char buf[chunk];
    int count = 0; int currLineHasChar = 0;
    ssize_t r;

    while((r = read(fd, buf, chunk)) > 0){
        for(ssize_t i = 0; i < r; i++){
            if(buf[i] == '\n'){
                if(currLineHasChar) count++;
                currLineHasChar = 0;
            }
            else currLineHasChar = 1;
        }
    }
    if(currLineHasChar) count++;

    return count;
}

void peekFwd(int fd, int* lineNum){
    int chunk = 4096;

    char buf[chunk];
    char* line = NULL;
    int lineLen = 0, lineCap = 0;
    ssize_t r;

    while((r = read(fd, buf, chunk)) > 0){
        for(ssize_t i = 0; i < r; i++){
            if(buf[i] == '\n'){
                if(lineLen == 0) printf("\n");
                else if(numbered){
                    (*lineNum)++;
                    printf("%d %.*s\n", *lineNum, lineLen, line);
                }
                else printf("%.*s\n", lineLen, line);

                lineLen = 0;
            }
            else{
                if(lineLen == lineCap){
                    lineCap = lineCap ? lineCap * 2 : 64;
                    line = (char*)realloc(line, lineCap);
                }
                line[lineLen++] = buf[i];
            }
        }
    }

    if(lineLen > 0){
        if(numbered){
            (*lineNum)++;
            printf("%d %.*s\n", *lineNum, lineLen, line);
        }
        else printf("%.*s\n", lineLen, line);
    }

    if(line) free(line);
}

void peekBufRev(int fd, int* lineNum){
    int chunk = 4096;
    int idx = 0;
    char* buf = (char*)malloc(chunk * sizeof(char));
    ssize_t r;

    while((r = read(fd, buf + idx, chunk - idx)) > 0){
        idx += r;
        if(idx == chunk){
            chunk *= 2;
            buf = (char*)realloc(buf, chunk);
        }
    }

    char** lines = NULL;
    int count = 0; int lineCap = 16;
    int j = 0;

    for(int i = 0; i < idx; i++){
        if(buf[i] == '\n'){
            if(count == lineCap){
                lineCap = lineCap * 2;
                lines = (char**)realloc(lines, lineCap * sizeof(char*));
            }

            lines[count] = (char*)malloc((i - j + 1) * sizeof(char) );
            strncpy(lines[count], buf + j, (i -j));
            lines[count][i - j] = '\0';
            count++;
            j = i + 1;
        }
    }
    if(j < idx){
        if(count == lineCap){
            lineCap = lineCap * 2;
            lines = (char**)realloc(lines, lineCap * sizeof(char*));
        }

        lines[count] = (char*)malloc((idx - j + 1) * sizeof(char));
        memcpy(lines[count], buf + j, idx - j);
        lines[count][idx - j] = '\0';
        count++;
    }
    free(buf);

    int* nums = NULL;
    if(numbered){
        nums = (int*)malloc(count * sizeof(int));
        int n = *lineNum;
        for(int i = 0; i < count; i++){
            if(strlen(lines[i]) > 0) nums[i] = ++n;
            else nums[i] = -1;
        }
        *lineNum = n;
    }

    for(int i = count - 1; i >= 0; i--){
        if(numbered && nums[i] != -1) printf("%d %s\n", nums[i], lines[i]);
        else printf("%s\n", lines[i]);
        free(lines[i]);
    }

    if(lines) free(lines);
    if(nums) free(nums);

    return;
}

void peekSeekRev(int fd, int* lineNum){
    int chunk = 4096;

    int fileNonEmpty = numbered ? countNonEmpty(fd) : 0;
    int curNum = *lineNum + fileNonEmpty;

    off_t pos = lseek(fd, 0, SEEK_END);
    char* carry = NULL;
    int carryLen = 0, first = 1;

    while(pos > 0){
        int take = (pos > chunk) ? chunk : (int)pos;
        pos -= take;

        lseek(fd, pos, SEEK_SET);
        char buf[chunk];
        read(fd, buf, take);

        int len = take + carryLen;
        char* temp = (char*)malloc(len * sizeof(char));
        strncpy(temp, buf, take);
        if(carryLen) strncpy(temp + take, carry, carryLen);
        if(carry) free(carry);
        carry = NULL; carryLen = 0;

        int idx = len;
        if(first){
            if(idx > 0 && temp[idx - 1] == '\n') idx--;
            first = 0;
        }

        while(1){
            if(idx == 0){
                if(pos == 0) printf("\n");
                idx = -1;
                break;
            }

            int j = idx - 1;
            while(j >= 0 && temp[j] != '\n'){
                j--;
            }

            if(j < 0){
                if(pos == 0){
                    if(numbered){ 
                        printf("%d %.*s\n", curNum, idx, temp); 
                        curNum--; 
                    }
                    else{ 
                        printf("%.*s\n", idx, temp);
                    }
                }
                else{
                    carry = (char*)malloc(idx);
                    strncpy(carry, temp, idx);
                    carryLen = idx;
                }
                idx = -1;
                break;
            }

            int lstart = j + 1;
            int llen = idx - lstart;

            if(llen == 0) printf("\n");
            else if(numbered){ printf("%d %.*s\n", curNum, llen, temp + lstart); curNum--; }
            else printf("%.*s\n", llen, temp + lstart);

            idx = j;
        }

        free(temp);
    }

    if(numbered) *lineNum += fileNonEmpty;
}

int main(int argc, char* argv[]){
    numbered = 0;
    reversed = 0;

    int idx = 1;
    while(idx<argc){
        char* token = argv[idx];

        if(token[0] == '-' && (int)strlen(token) > 1 && token[1] != '/'){
            for(int j = 1; token[j] != '\0'; j++){
                if(token[j] == 'n') numbered = 1;
                else if(token[j] == 'r') reversed = 1;
                else{
                    printf("peek: invalid syntax\n");
                    return 1;
                }
            }
        }
        else{
            break;
        }
    }

    int lineNum = 0;

    if(idx == argc){
        if(reversed) peekBufRev(STDIN_FILENO, &lineNum);
        else peekFwd(STDIN_FILENO, &lineNum);
        return 0;
    }

    while(idx<argc){
        char* token = argv[idx];

        if(!strcmp(token, "-")){
            if(reversed) peekBufRev(STDIN_FILENO, &lineNum);
            else peekFwd(STDIN_FILENO, &lineNum);
            
            idx++;
            continue;
        }

        struct stat st;
        if(stat(token, &st) != 0){
            printf("peek: no such file or directory\n");
            return 1;
        }
        if(S_ISDIR(st.st_mode)){
            printf("peek: is a directory\n");
            return 1;
        }

        int fd = open(token, O_RDONLY);
        if(fd < 0){
            printf("peek: no such file or directory\n");
            return 1;
        }

        if(reversed){
            if(S_ISREG(st.st_mode)) peekSeekRev(fd, &lineNum);
            else peekBufRev(fd, &lineNum);
        }
        else{
            peekFwd(fd, &lineNum);
        }

        close(fd);

        idx++;
    }

    return 0;
}