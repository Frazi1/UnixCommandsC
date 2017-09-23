#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include "command.c"

// cat log.txt | grep 18/Oct/2006 | cut -d \" -f4 | grep -v -x "-" | sort | uniq -c | sort -nrk 1 | head -10
int main(int argc, char *argv[]) {
//    if (argc < 2)
//        return -1;

    char* pathToFile = "log.txt";

    const char *cat[] = {"cat", pathToFile, 0};
    const char *grep1[] = {"grep", "18/Oct/2006", 0};
    const char *cut[] = {"cut", "--delimiter=\"", "-f4", 0};
    const char *grep2[] = {"grep", "-v", "-x", "-", 0};
    const char *sort1[] = {"sort", 0};
    const char *uniq[] = {"uniq", "-c", 0};
    const char *sort2[] = {"sort", "-nrk", "1", 0};
    const char *head[] = {"head", "-10", 0};

    struct command cmd[] = { {cat}, {grep1}, {cut}, {grep2}, {sort1}, {uniq}, {sort2}, {head} };

    int fd[2];
    pipe(fd);

    pid_t pid1 = fork_pipes(8, cmd, fd[1]);
    waitpid(pid1, NULL, 0);

    pid_t pid = fork();

    if (!pid) {
        char result[1024];
        read(fd[0], result, 1024);
        printf(result);
    }

    waitpid(pid, NULL, 0);
    return 0;
}

int spawn_proc(int in, int out, struct command *cmd) {
    pid_t pid;

    if ((pid = fork()) == 0) {
        if (in != 0) {
            dup2(in, 0);
            close(in);
        }
        if (out != 1) {
            dup2(out, 1);
            close(out);
        }

        return execvp(cmd->argv[0], (char * const *)cmd->argv);
    }
    return pid;
}

int fork_pipes(int n, struct command *cmd, int out) {
    int i;
    pid_t pid;
    int in, fd[2];

    in = 0;

    for (i = 0; i < n - 1; ++i) {
        pipe(fd);

        spawn_proc(in, fd[1], cmd + i);

        close(fd[1]);

        in = fd[0];
    }

    pid = fork();

    if (!pid) {
        if (in != 0)
            dup2(in, 0);
        if (out != 1) {
            dup2(out, 1);
            close(out);
        }
        execvp(cmd[i].argv[0], (char * const *)cmd[i].argv);
    }

    return pid;
}