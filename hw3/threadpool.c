// threadpool - a concurrent work queue

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

unsigned long djb2_hash(const char *str) {
unsigned long hash = 5381;
int c;
while ((c = *str++))
hash = ((hash << 5) + hash) + c;
return hash;
}

void process_task(const char *task) {
usleep(100000);
unsigned long hash = djb2_hash(task);
printf("%lu %s\n", hash, task);
fflush(stdout);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);

    if (num_threads < 1) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    char buf[1024];

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        char *newline = strchr(buf, '\n');
        if (newline) {
            *newline = '\0';
        }

        if (buf[0] == '\0') {
            continue;
        }

        process_task(buf);
    }
    
    return 0;
}