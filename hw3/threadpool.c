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

typedef struct task_node {
char task[1024];
struct task_node *next;
} task_node_t;

typedef struct {
task_node_t *head;
task_node_t *tail;
int done;
pthread_mutex_t mutex;
pthread_cond_t cond;
int count;
int current_workers;
int base_workers;
} task_queue_t;

task_queue_t queue = { .head = NULL, .tail = NULL, .done = 0, .count = 0, .current_workers = 0, .base_workers = 0 };
void push_task(task_queue_t *q, const char *task) {
    task_node_t *node = malloc(sizeof(task_node_t));
    strcpy(node->task, task);
    node->next = NULL;
    if (q->tail) {
        q->tail->next = node;
        q->tail = node;
    } else {
        q->head = node;
        q->tail = node;
    }
    q->count++;
}

int pop_task(task_queue_t *q, char *buf) {
    if (q->head == NULL) return 0;
    task_node_t *node = q->head;
    strcpy(buf, node->task);
    q->head = node->next;
    if (q->head == NULL) q->tail = NULL;
    free(node);
    q->count--;
    return 1;
}
void *worker(void *arg) {
    int id = *(int *)arg;
    char buf[1024];
    while (1) {
        pthread_mutex_lock(&queue.mutex);
        while (queue.head == NULL && !queue.done) {
            if (queue.current_workers > queue.base_workers) {
                queue.current_workers--;
                pthread_mutex_unlock(&queue.mutex);
                return NULL;
            }
            pthread_cond_wait(&queue.cond, &queue.mutex);
        }
        if (pop_task(&queue, buf)) {
            pthread_mutex_unlock(&queue.mutex);
            process_task(buf);
        } else if (queue.done) {
            pthread_mutex_unlock(&queue.mutex);
            break;
        } else {
            pthread_mutex_unlock(&queue.mutex);
        }
    }
    return NULL;
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

    pthread_mutex_init(&queue.mutex, NULL);
    pthread_cond_init(&queue.cond, NULL);
    
    int max_threads = num_threads * 10;
    pthread_t *threads = malloc(sizeof(pthread_t) * max_threads);
    int *thread_ids = malloc(sizeof(int) * max_threads);
    int total_threads = num_threads;

    queue.base_workers = num_threads;
    queue.current_workers = num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &thread_ids[i]);
    }
    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        char *newline = strchr(buf, '\n');
        if (newline) *newline = '\0';
        if (buf[0] == '\0') continue;
        pthread_mutex_lock(&queue.mutex);
        push_task(&queue, buf);
        pthread_cond_signal(&queue.cond);
        if (queue.count > queue.base_workers * 2 && 
            queue.current_workers < max_threads) {
            thread_ids[total_threads] = total_threads;
            pthread_create(&threads[total_threads], NULL, worker, &thread_ids[total_threads]);
            queue.current_workers++;
            total_threads++;
        }
        pthread_mutex_unlock(&queue.mutex);
    }

    pthread_mutex_lock(&queue.mutex);
    queue.done = 1;
    pthread_cond_broadcast(&queue.cond);
    pthread_mutex_unlock(&queue.mutex);

    for (int i = 0; i < total_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.cond);
    free(threads);
    free(thread_ids);

    return 0;
}