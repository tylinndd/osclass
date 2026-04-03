// mysh - a simple Unix shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main(void) {
    char line[1024];
    char *args[64];
    int arg_count;
    while (1) {
        printf("mysh> ");
        fflush(stdout);
        
        if (fgets(line, sizeof(line), stdin)== NULL) {
            printf("\n");
            break;
        }

        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        arg_count = 0;
        args[arg_count] = strtok(line, " ");
        while (args[arg_count] != NULL) {
            arg_count++;
            args[arg_count] = strtok(NULL, " ");
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL){
                int change = chdir(args[1]);
                if (change == -1){
                    perror("cd failed");
                } else{
                    chdir(args[1]);
                }
            } else {
                chdir(getenv("HOME"));
            }
            continue;
        }

        if (strcmp(args[0],"exit") == 0){
            break;
        }

        char *fname = NULL;
        char *fname_in = NULL;
        char **left = NULL;
        char **right = NULL;
        for (int x = 0; args[x] != NULL; x++) {
            if (strcmp(args[x], ">") == 0) {
                fname = args[x + 1];
                args[x] = NULL;
                args[x+1] = NULL;
                
            } else if (strcmp(args[x], "<") == 0) {
                fname_in = args[x + 1];
                args[x] = NULL;
                args[x+1] = NULL;
            } else if (strcmp(args[x], "|") == 0){
                args[x] = NULL;
                left = args;
                right = &args[x + 1];
            }
        }


        
        pid_t pid = fork();
        if (pid == 0){
            if (fname != NULL){
                int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1){
                    perror("Could not open file");
                    exit(1);
                }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            }

            if (fname_in != NULL){
                int fd = open(fname_in, O_RDONLY);
                if (fd == -1){
                    perror("Could not open file");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);

            }
            if (left != NULL) {
                int pipefd[2];
                pipe(pipefd);
                pid_t pid_l = fork();
                
                if (pid_l == 0){
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                    execvp(left[0], left);
                    perror("left command fail");
                    exit(1); 
                }
                
                pid_t pid_r = fork();
                if (pid_r == 0){
                    close(pipefd[1]);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    execvp(right[0], right);
                    perror("right command fail");
                    exit(1);
                }
                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid_l,NULL,0);
                waitpid(pid_r, NULL, 0);
                exit(0);
                
            }
            
            execvp(args[0], args);
            perror("execvp failed");
            exit(1);
        } else if (pid > 0){
            waitpid(pid, NULL, 0);
        } else {
            perror("fork failed");
        }
        printf("You entered: %s\n", line);
        

    }
    return 0;
}