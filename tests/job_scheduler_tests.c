#include <pthread.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar1, cvar2, count_nonzero;

unsigned count = 0;

JobScheduler js = NULL;

void *decrement(void *argp);

void *increment(void *argp);

void create_job_scheduler(void) {
    js_create(&js, 0);
    TEST_CHECK(js != NULL);
}

void submit_jobs(void) {
    //TEST_CHECK(js_submit_job(js, js_create_job(increment, NULL)));
    //TEST_CHECK(js_submit_job(js, js_create_job(decrement, NULL)));
}

void execute_all_jobs(void) {
    TEST_CHECK(js_execute_all_jobs(js));
    TEST_CHECK(count == 0);
}

void wait_all_jobs(void) {
    TEST_CHECK(js_wait_all_jobs(js));
}

void destroy_job_scheduler(void) {
    //js_destroy(&js);
    TEST_CHECK(js == NULL);
}

TEST_LIST = {
        {"create_job_scheduler",  create_job_scheduler},
        {"submit_jobs",           submit_jobs},
        {"execute_all_jobs",      execute_all_jobs},
        {"wait_all_jobs",         wait_all_jobs},
        {"destroy_job_scheduler", destroy_job_scheduler},
        {NULL, NULL}
};

/*** Thread functions ***/
/*used: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(void *argp) {
    sleep(1);
    pthread_mutex_lock(&mtx);
    printf("Thread %ld: start decrement_count %d\n", pthread_self(), count);
    while (count == 0) {
        pthread_cond_wait(&count_nonzero, &mtx);
    }
    count = count - 1;
    printf("Thread %ld: end decrement_count %d\n", pthread_self(), count);
    pthread_mutex_unlock(&mtx);
    return NULL;
}

void *increment(void *argp) {
    sleep(1);
    pthread_mutex_lock(&mtx);
    printf("Thread %ld: start increment_count %d\n", pthread_self(), count);
    if (count == 0) {
        pthread_cond_signal(&count_nonzero);
    }
    count = count + 1;
    printf("Thread %ld: ent increment_count %d\n", pthread_self(), count);
    pthread_mutex_unlock(&mtx);
    return NULL;
}