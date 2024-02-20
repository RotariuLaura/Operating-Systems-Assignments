#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>

sem_t sem1, sem2, sem3, sem4;

void* thread_function(void *param)
{
    int id = *(int*)param;
    if(id == 3)
    {
        info(BEGIN, 3, id);
        sem_post(&sem1);
        sem_wait(&sem2);
        info(END, 3, id);
    }
    if(id == 1)
    {
        sem_wait(&sem1);
        info(BEGIN, 3, id);
        info(END, 3, id);
        sem_post(&sem2);
    }
    if(id == 4)
    {   
        sem_wait(&sem3);
        info(BEGIN, 3, id);
        info(END, 3, id);
        sem_post(&sem4);
    }
    if(id == 2)
    {
        info(BEGIN, 3, id);
        info(END, 3, id);
    }
    pthread_exit(NULL);
}

void* thread_function_2_5(void *param)
{
    int id = *(int*)param;
    if(id == 3)
    {
        info(BEGIN, 9, id);
        info(END, 9, id);
        sem_post(&sem3);
    }
    if(id == 2)
    {
        sem_wait(&sem4);
        info(BEGIN, 9, id);
        info(END, 9, id);
    }
    if(id == 1 || id == 4 || id == 5 || id == 6)
    {
        info(BEGIN, 9, id);
        info(END, 9, id);
    }
    pthread_exit(NULL);
}

pthread_mutex_t mutex;
pthread_cond_t cond;
int running_threads = 0;

void* thread_function_2_4(void *param)
{
    int id = *(int*)param;
    pthread_mutex_lock(&mutex);
    while(running_threads >= 5)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    running_threads++;
    pthread_mutex_unlock(&mutex);
    info(BEGIN, 7, id);
    info(END, 7, id);
    pthread_mutex_lock(&mutex);
    running_threads--;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main()
{
    init();
    pid_t pid2=-1, pid3=-1, pid4=-1, pid5=-1, pid6=-1, pid7=-1, pid8=-1, pid9=-1;
    info(BEGIN, 1, 0);
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 1);
    sem_init(&sem4, 0, 1);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pid2 = fork();
    if(pid2 == 0)
    {
        info(BEGIN, 2, 0);
        pid4 = fork();
        if(pid4 == 0)
        {
            info(BEGIN, 4, 0);
            pid7 = fork();
            if(pid7 == 0)
            {
                info(BEGIN, 7, 0);
                pid9 = fork();
                if(pid9 == 0)
                {
                    pthread_t threads[6];
                    int ids[] = {1, 2, 3, 4, 5, 6};
                    info(BEGIN, 9, 0);
                    for(int i = 0; i < 6; i++)
                    {
                        pthread_create(&threads[i], NULL, thread_function_2_5, &ids[i]);
                    }
                    for(int i = 0; i < 6; i++)
                    {
                        pthread_join(threads[i], NULL);
                    }
                    info(END, 9, 0);
                    exit(9);
                }
                else
                {
                    pthread_t threads[38];
                    int ids[38];
                    for(int i = 0; i < 38; i++)
                    {
                        ids[i] = i + 1;
                    }
                    for(int i = 0; i < 38; i++)
                    {
                        pthread_create(&threads[i], NULL, thread_function_2_4, &ids[i]);
                    }
                    for(int i = 0; i < 38; i++)
                    {
                        pthread_join(threads[i], NULL);
                    }
                    waitpid(pid9, NULL, 0);
                    info(END, 7, 0);
                    exit(7);
                }
            }
            else
            {
                waitpid(pid7, NULL, 0);
                info(END, 4, 0);
                exit(4);
            }
        }
        else
        {
            waitpid(pid4, NULL, 0);
            info(END, 2, 0);
            exit(2);
        }
    }
    else
    {
        pid3 = fork();
        if(pid3 == 0)
        {
            pthread_t threads[4];
            int ids[] = {1, 2, 3, 4};
            info(BEGIN, 3, 0);
            for(int i = 0; i < 4; i++)
            {
                pthread_create(&threads[i], NULL, thread_function, &ids[i]);
            }
            for(int i = 0; i < 4; i++)
            {
                pthread_join(threads[i], NULL);
            }
            info(END, 3, 0);
            exit(3);
        }
        else
        {
            pid5 = fork();
            if(pid5 == 0)
            {
                info(BEGIN, 5, 0);
                info(END, 5, 0);
                exit(5);
            }
            else
            {
                pid6 = fork();
                if(pid6 == 0)
                {
                    info(BEGIN, 6, 0);
                    pid8 = fork();
                    if(pid8 == 0)
                    {
                        info(BEGIN, 8, 0);
                        info(END, 8, 0);
                        exit(8);
                    }
                    else
                    {
                        waitpid(pid8, NULL, 0);
                        info(END, 6, 0);
                        exit(6);
                    }
                }
                else
                {
                    waitpid(pid2, NULL, 0);
                    waitpid(pid3, NULL, 0);
                    waitpid(pid5, NULL, 0);
                    waitpid(pid6, NULL, 0);
                    info(END, 1, 0);
                    exit(1);
                }
            }
        }
    }
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);
    sem_destroy(&sem4);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
