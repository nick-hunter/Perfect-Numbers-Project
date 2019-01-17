#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

//Nicholas Hunter

//#define DEBUG 1
#define ARR_SIZE 10
#define REASONABLE_THREAD_MAX 200

 struct shared_data {
    pthread_mutex_t lock;
    unsigned long long int *factors;
    unsigned int position;
    size_t size;
};

struct thread_data {
    unsigned long long int n;
    unsigned long long int begin;
    unsigned long long int end;
    struct shared_data *shared_data;
};

void *thread(void* thread_data_ptr);

int main(int argc, char *argv[]){
    if(argc != 3){
        printf("Usage ./program [number to check] [number of threads]\n");
        return 1;
    }

    unsigned int num_threads = atoi(argv[2]);
    unsigned long long int n = strtoull(argv[1], (char **)NULL, 10);

    struct shared_data shared;
    shared.factors = (unsigned long long int *)malloc(ARR_SIZE * sizeof(unsigned long long int));
    shared.position = 0;
    shared.size = ARR_SIZE;
    pthread_mutex_init(&shared.lock, NULL);

    if(num_threads > REASONABLE_THREAD_MAX){
        fprintf(stderr, "Max number of threads is %d\n", REASONABLE_THREAD_MAX);
        return 1;
    }

    //Pointers to threads
    pthread_t *threads = malloc(sizeof(pthread_t)*num_threads);

    //Structs to pass to the threads
    struct thread_data *data = malloc(sizeof(struct thread_data)*num_threads);

    //Too many threads too little work
    if(num_threads > n/2){
        num_threads = (int)n/2;
    }

    //https://math.stackexchange.com/q/46014
    unsigned long long int default_to_assign = floor((n/2)/num_threads);
    unsigned long long int num_assigned;
    unsigned long long int start = 0;
    unsigned long long int end = 0;
    int remainder = (n/2)%num_threads;

    for(int i = 0; i < num_threads; i++){
        if(i < remainder){
            num_assigned = default_to_assign + 1;
        }else{
            num_assigned = default_to_assign;
        }
        start = end + 1;
        end = end + num_assigned;

        data[i].n = n;
        data[i].begin = start;
        data[i].end = end;
        data[i].shared_data = &shared;

        #ifdef DEBUG
          printf("thread %d assigned %llu to %llu (%llu)\n", i, start, end, num_assigned);
        #endif

        pthread_create(&threads[i], NULL, thread, &data[i]);
    }

    for(int i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    unsigned long long int sum = 0;
    for(int i = 0; i < shared.position; i++){
        sum += shared.factors[i];
    }

    free(shared.factors);

    if(sum == n){
        printf("%llu is a perfect number\n", n);
    }else{
        printf("%llu is not a perfect number\n", n);
    }

    return 0;
}

void *thread(void* thread_data_ptr){
    struct thread_data *thread_data = thread_data_ptr;

    for(unsigned long long int i = thread_data->begin; i <= thread_data->end; i++){
        if(thread_data->n%i == 0){
            unsigned int *position = &(thread_data->shared_data->position);
            size_t *size = &(thread_data->shared_data->size);

            pthread_mutex_lock(&(thread_data->shared_data->lock));
            if(*position == *size - 1){
                thread_data->shared_data->factors = realloc(thread_data->shared_data->factors, \
                  *size * 2 * sizeof(unsigned long long int));
                *size *= 2;
                if(thread_data->shared_data->factors == NULL){
                    fprintf(stderr, "realloc failed.\n");
                    exit(EXIT_FAILURE);
                }
            }
            thread_data->shared_data->factors[thread_data->shared_data->position] = i;
            thread_data->shared_data->position++;
            pthread_mutex_unlock(&(thread_data->shared_data->lock));
            printf("%llu\n", i);
        }
    }
    return NULL;
}
