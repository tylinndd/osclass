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
        fgets(line, sizeof(line), stdin);
        char buffer[100];
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("\n");
            break;
        }

        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        printf("You entered: %s\n", line);
    }
    return 0;
}