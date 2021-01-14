#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>

#include "../include/queue.h"

struct queue_t {
    int front;
    int rear;
    int counter;
    int buf_sz;
    int type_sz;
    sem_t sem_mutex;
    sem_t sem_non_empty;
    sem_t sem_non_full;
    pthread_mutex_t mutex;
    pthread_cond_t cond_non_empty;
    pthread_cond_t cond_non_full;
    char buffer[];
};

/***Private functions***/

void enqueue(Queue q, void *item) {
    q->counter++;
    memcpy(q->buffer + (q->rear * q->type_sz), item, q->type_sz);
    q->rear = (q->rear + 1) % q->buf_sz;
}

void dequeue(Queue q, void *item) {
    q->counter--;
    memcpy(item, q->buffer + (q->front * q->type_sz), q->type_sz);
    memset(q->buffer + (q->front * q->type_sz), 0, q->type_sz);
    q->front = (q->front + 1) % q->buf_sz;
}

/***Public functions***/

void queue_create(Queue *q, int buf_sz, int type_sz) {
    assert(*q == NULL);
    *q = (Queue) malloc(sizeof(struct queue_t) + buf_sz * type_sz);
    assert(*q != NULL);
    (*q)->buf_sz = buf_sz;
    (*q)->type_sz = type_sz;
    (*q)->counter = 0;
    (*q)->front = 0;
    (*q)->rear = 0;
    memset((*q)->buffer, 0, (*q)->buf_sz * (*q)->type_sz);

    sem_init(&(*q)->sem_mutex, 0, 1);
    sem_init(&(*q)->sem_non_empty, 0, 0);
    sem_init(&(*q)->sem_non_full, 0, buf_sz);

    pthread_mutex_init(&(*q)->mutex, NULL);
    pthread_cond_init(&(*q)->cond_non_empty, NULL);
    pthread_cond_init(&(*q)->cond_non_full, NULL);
}

void queue_destroy(Queue *q, void (*free_t)(void *)) {
    if (free_t != NULL) {
        void *item = malloc((*q)->type_sz);
        if (free_t != NULL) {
            while (queue_dequeue(*q, item, true)) {
                free_t(item);
            }
        }
        free(item);
    }
    free(*q);
    *q = NULL;
}

bool queue_is_empty(Queue q, const bool sync) {
    bool is_empty = false;
    //sem_wait(&q->sem_mutex);
    if (sync) {
        pthread_mutex_lock(&q->mutex);
        //sem_wait(&q->sem_mutex);
    }
    is_empty = (q->counter <= 0);
    if (sync) {
        pthread_mutex_unlock(&q->mutex);
        //sem_post(&q->sem_mutex);
    }
    //sem_post(&q->sem_mutex);
    return is_empty;
}

int queue_size(Queue q) {
    return q->counter;
}

bool queue_is_full(Queue q, const bool sync) {
    bool is_full = false;
    //sem_wait(&q->sem_mutex);
    if (sync) {
        pthread_mutex_lock(&q->mutex);
        //sem_wait(&q->sem_mutex);
    }
    is_full = (q->counter == q->buf_sz);
    if (sync) {
        pthread_mutex_unlock(&q->mutex);
        //sem_post(&q->sem_mutex);
    }
    //sem_post(&q->sem_mutex);
    return is_full;
}

bool queue_enqueue(Queue q, void *item, bool sync) {
    int ret = 0;
    if (sync) {
        //ret += sem_wait(&q->sem_non_full);
        //ret += sem_wait(&q->sem_mutex);
        ret += pthread_mutex_lock(&q->mutex);
        while (queue_is_full(q, false)) {
            ret += pthread_cond_wait(&q->cond_non_full, &q->mutex);
        }

        /** critical section **/
        enqueue(q, item);
        /*********************/

        ret += pthread_mutex_unlock(&q->mutex);
        ret += pthread_cond_signal(&q->cond_non_empty);
        //ret += sem_post(&q->sem_mutex);
        //ret += sem_post(&q->sem_non_empty);
    } else {
        if (!queue_is_full(q, false)) {
            enqueue(q, item);
        } else {
            return false;
        }
    }
    return !ret;
}

bool queue_dequeue(Queue q, void *item, bool sync) {
    int ret = 0;
    if (sync) {
        //ret += sem_wait(&q->sem_non_empty);
        //ret += sem_wait(&q->sem_mutex);
        ret += pthread_mutex_lock(&q->mutex);
        while (queue_is_empty(q, false)) {
            ret += pthread_cond_wait(&q->cond_non_empty, &q->mutex);
        }

        /** critical section **/
        dequeue(q, item);
        /*********************/

        ret += pthread_mutex_unlock(&q->mutex);
        ret += pthread_cond_signal(&q->cond_non_full);
        //ret += sem_post(&q->sem_mutex);
        //ret += sem_post(&q->sem_non_full);
    } else {
        if (!queue_is_empty(q, false)) {
            dequeue(q, item);
        } else {
            return false;
        }
    }
    return !ret;
}

void queue_unblock_enqueue(Queue q) {
    pthread_cond_signal(&q->cond_non_full);
    //sem_post(&q->sem_non_full);
}

void queue_unblock_dequeue(Queue q) {
    pthread_cond_broadcast(&q->cond_non_empty);
    //sem_post(&q->sem_non_empty);
}

void queue_inspect_by_order(Queue q, void *(*print_v)(void *)) {
    int current = q->front;
    if (!queue_is_empty(q, false)) {
        do {
            print_v(&(q->buffer[current]));
            printf("-");
            current = (current + 1) % q->buf_sz;
        } while (current != q->rear);
        printf("\n");
    }
}

void queue_inspect_by_array(Queue q, void *(*print_v)(void *)) {
    int i;
    for (i = 0; i < q->buf_sz; i++) {
        //CustWriteValue(stdout, &(q->array[i]));
        print_v(&(q->buffer[i]));
        printf("-");
    }
    printf("\n");
}
