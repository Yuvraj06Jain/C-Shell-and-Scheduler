#include "functions.h"

int spy(Node* args, Node* end){
    Node* temp = args;
    char pid_str[32];
    
    if (temp == end) {
        sprintf(pid_str, "%d", getpid());
    } else {
        strncpy(pid_str, temp->token, sizeof(pid_str)-1);
        pid_str[sizeof(pid_str)-1] = '\0';
        temp = temp->next;
        if (temp != end) {
            printf("spy: invalid syntax\n");
            return 1;
        }
    }
    
    for (int i = 0; pid_str[i] != '\0'; i++) {
        if (pid_str[i] < '0' || pid_str[i] > '9') {
            printf("spy: no such process\n");
            return 1;
        }
    }
    
    char proc_path[256];
    snprintf(proc_path, sizeof(proc_path), "/proc/%s", pid_str);
    
    struct stat st;
    if (stat(proc_path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        printf("spy: no such process\n");
        return 1;
    }
    
    char fd_path[256];
    snprintf(fd_path, sizeof(fd_path), "/proc/%s/fd", pid_str);
    if (access(fd_path, R_OK | X_OK) != 0) {
        if (errno == EACCES || errno == EPERM) {
            printf("spy: permission denied\n");
        } else {
            printf("spy: no such process\n");
        }
        return 1;
    }
    
    printf("PID\tFD\tTYPE\tPATH\n");

    char path[1024];
    char target[1024];
    ssize_t len;
    
    snprintf(path, sizeof(path), "/proc/%s/cwd", pid_str);
    len = readlink(path, target, sizeof(target) - 1);
    if (len != -1) {
        target[len] = '\0';
        printf("%s\t%s\t%s\t%s\n", pid_str, "cwd", "DIR", target);
    }
    

    char exe_path[1024] = "";
    snprintf(path, sizeof(path), "/proc/%s/exe", pid_str);
    len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        printf("%s\t%s\t%s\t%s\n", pid_str, "txt", "REG", exe_path);
    }
    

    snprintf(path, sizeof(path), "/proc/%s/maps", pid_str);
    FILE* maps = fopen(path, "r");
    if (maps) {
        char line[2048];
        char mapped_path[1024];
        char** printed_maps = malloc(1024 * sizeof(char*));
        int printed_count = 0;
        
        while (fgets(line, sizeof(line), maps)) {
            mapped_path[0] = '\0';
            if (sscanf(line, "%*s %*s %*s %*s %*s %1023[^\n]", mapped_path) == 1) {
                if (mapped_path[0] == '/') {
                    if (strcmp(mapped_path, exe_path) == 0) continue;
                    
                    int duplicate = 0;
                    for (int i = 0; i < printed_count; i++) {
                        if (strcmp(printed_maps[i], mapped_path) == 0) {
                            duplicate = 1;
                            break;
                        }
                    }
                    if (!duplicate) {
                        if (printed_count < 1024) {
                            printed_maps[printed_count++] = strdup(mapped_path);
                        }
                        printf("%s\t%s\t%s\t%s\n", pid_str, "mem", "REG", mapped_path);
                    }
                }
            }
        }
        for (int i = 0; i < printed_count; i++) {
            free(printed_maps[i]);
        }
        free(printed_maps);
        fclose(maps);
    }
    

    snprintf(path, sizeof(path), "/proc/%s/fd", pid_str);
    DIR* dir = opendir(path);
    if (dir) {
        struct dirent* entry;
        int fds[1024];
        int fd_count = 0;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
                if (fd_count < 1024) {
                    fds[fd_count++] = atoi(entry->d_name);
                }
            }
        }
        closedir(dir);
        

        for (int i = 0; i < fd_count - 1; i++) {
            for (int j = i + 1; j < fd_count; j++) {
                if (fds[i] > fds[j]) {
                    int t = fds[i]; fds[i] = fds[j]; fds[j] = t;
                }
            }
        }
        
        for (int i = 0; i < fd_count; i++) {
            char fd_str[32];
            snprintf(fd_str, sizeof(fd_str), "%d", fds[i]);
            snprintf(path, sizeof(path), "/proc/%s/fd/%d", pid_str, fds[i]);
            len = readlink(path, target, sizeof(target) - 1);
            if (len != -1) {
                target[len] = '\0';
                struct stat fd_st;
                char type_str[10] = "REG";
                if (stat(path, &fd_st) == 0) {
                    if (S_ISDIR(fd_st.st_mode)) strcpy(type_str, "DIR");
                    else if (S_ISCHR(fd_st.st_mode)) strcpy(type_str, "CHR");
                    else if (S_ISFIFO(fd_st.st_mode)) strcpy(type_str, "FIFO");
                    else if (S_ISSOCK(fd_st.st_mode)) strcpy(type_str, "SOCK");
                }
                printf("%s\t%s\t%s\t%s\n", pid_str, fd_str, type_str, target);
            }
        }
    }
    
    return 0;
}