#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>

int main() {
    char input[256];
    char *args[32];
    char hostname[256];
    char cwd[1024];
    uid_t uid;
    struct passwd *pw;
    char *username;

    // 获取主机名
    gethostname(hostname, sizeof(hostname));

    while (1) {
        // 获取真实用户信息
        uid = getuid();
        pw = getpwuid(uid);
        username = (pw != NULL) ? pw->pw_name : "unknown";
        
        // 获取当前工作目录
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            strcpy(cwd, "?");
        }

        // 根据用户权限显示不同的提示符
        if (uid == 0) {
            // root用户：用户名@主机名 红色，路径白色
            printf("\033[1;31m%s@%s\033[0m:\033[1;37m%s#\033[0m ", username, hostname, cwd);
        } else {
            // 普通用户：用户名@主机名 绿色，路径白色  
            printf("\033[1;32m%s@%s\033[0m:\033[1;37m%s$\033[0m ", username, hostname, cwd);
        }

        // 读取命令
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        // 移除换行符
        input[strcspn(input, "\n")] = '\0';

        // 跳过空命令
        if (strlen(input) == 0) {
            continue;
        }

        // 🔥 新增：跳过注释行（以 # 开头）
        if (input[0] == '#') {
            continue;
        }

        // 解析命令
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < 31) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        // 检查是否解析到有效命令
        if (args[0] == NULL) {
            continue;  // 没有有效命令，重新循环
        }

        // 处理内置命令
        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0) {
            break;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                // cd 无参数时回到家目录
                chdir(getenv("HOME"));
            } else if (chdir(args[1]) != 0) {
                printf("hsh: cd: %s: No such file or directory\n", args[1]);
            }
            continue;
        }

        // 执行外部命令
        pid_t pid = fork();

        if (pid == 0) {
            // 子进程
            execvp(args[0], args);
            // 如果execvp失败
            printf("hsh: command not found: %s\n", args[0]);
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // 父进程
            int status;
            waitpid(pid, &status, 0);
        } else {
            printf("hsh: fork failed\n");
        }
    }
    return 0;
}
