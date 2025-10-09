#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>

// 简单的 help 命令 - 显示 /usr/bin 目录内容
void show_help() {
      printf("            GNU hish, version 2.00.21(1)-release\n");
    printf("这些 shell 命令是外部定义的。键入 `help' 以查看此列表.\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // 子进程执行 ls 命令
        execlp("ls", "ls", "/usr/bin", NULL);
        // 如果 execlp 失败
        printf("-hish: 列出失败... /usr/bin\n");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // 父进程等待子进程结束
        int status;
        waitpid(pid, &status, 0);
    } else {
        printf("-hish: fork失败...\n");
    }
}

int main() {
    char input[256];
    char *args[32];
    char hostname[256];
    char cwd[1024];
    uid_t uid;
    struct passwd *pw;
    char *username;

    gethostname(hostname, sizeof(hostname));

    while (1) {
        uid = getuid();
        pw = getpwuid(uid);
        username = (pw != NULL) ? pw->pw_name : "unknown";
        
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "?");
        }

        if (uid == 0) {
            printf("\033[1;31m%s@%s\033[0m:\033[1;37m%s#\033[0m ", username, hostname, cwd);
        } else {
            printf("\033[1;32m%s@%s\033[0m:\033[1;37m%s$\033[0m ", username, hostname, cwd);
        }

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        if (input[0] == '#') {
            continue;
        }

        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < 31) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0) {
            break;
        }

        // help 命令 - 直接显示 /usr/bin 内容
        if (strcmp(args[0], "help") == 0) {
            show_help();
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                chdir(getenv("HOME"));
            } else if (chdir(args[1]) != 0) {
                printf("-hish: cd: %s: 没有那个文件或目录...\n", args[1]);
            }
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            printf("-hish: 命令未找到... : %s\n", args[0]);
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            printf("-hish: fork失败...\n");
        }
    }
    return 0; 
}
