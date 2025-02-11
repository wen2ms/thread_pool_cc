#include "thread_pool.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

template <class T>
ThreadPool<T>::ThreadPool(int min_num, int max_num) {
    do {
        task_queue_ = new TaskQueue<T>;

        if (task_queue_ == nullptr) {
            printf("new task_queue_ failed...\n");
            break;
        }

        thread_ids_ = new pthread_t[max_num];

        if (thread_ids_ == nullptr) {
            printf("new thread_ids failed...\n");
            break;
        }

        memset(thread_ids_, 0, sizeof(pthread_t) * max_num);

        min_num_ = min_num;
        max_num_ = max_num;
        busy_num_ = 0;
        alive_num_ = min_num;
        exit_num_ = 0;

        if (pthread_mutex_init(&mutex_pool_, NULL) != 0 || pthread_cond_init(&not_empty_, NULL) != 0) {
            printf("mutex or condition init failed...\n");
            break;
        }

        shotdown_ = false;

        pthread_create(&manager_id_, NULL, manager, this);
        for (int i = 0; i < min_num; ++i) {
            pthread_create(&thread_ids_[i], NULL, worker, this);
        }

        return;
    } while (false);

    if (thread_ids_) {
        delete[] thread_ids_;
    }

    if (task_queue_) {
        delete task_queue_;
    }
}

template <class T>
ThreadPool<T>::~ThreadPool() {
    shotdown_ = true;
    pthread_join(manager_id_, NULL);

    for (int i = 0; i < alive_num_; ++i) {
        pthread_cond_signal(&not_empty_);
    }

    if (task_queue_) {
        delete task_queue_;
        task_queue_ = nullptr;
    }

    if (thread_ids_) {
        delete[] thread_ids_;
        thread_ids_ = nullptr;
    }

    pthread_mutex_destroy(&mutex_pool_);

    pthread_cond_destroy(&not_empty_);
}

template <class T>
void ThreadPool<T>::add_task(Task<T> task) {
    if (shotdown_) {
        return;
    }

    task_queue_->add_task(task);

    pthread_cond_signal(&not_empty_);
}

template <class T>
int ThreadPool<T>::get_busy_num() {
    pthread_mutex_lock(&mutex_pool_);
    int busy_num = busy_num_;
    pthread_mutex_unlock(&mutex_pool_);

    return busy_num;
}

template <class T>
int ThreadPool<T>::get_alive_num() {
    pthread_mutex_lock(&mutex_pool_);
    int alive_num = alive_num_;
    pthread_mutex_unlock(&mutex_pool_);

    return alive_num;
}

template <class T>
void* ThreadPool<T>::manager(void* arg) {
    ThreadPool<T>* thread_pool = static_cast<ThreadPool<T>*>(arg);

    while (!thread_pool->shotdown_) {
        sleep(3);

        pthread_mutex_lock(&thread_pool->mutex_pool_);

        int queue_size = thread_pool->task_queue_->tasks_number();
        int alive_num = thread_pool->alive_num_;
        int busy_num = thread_pool->busy_num_;

        pthread_mutex_unlock(&thread_pool->mutex_pool_);

        if (queue_size > alive_num && alive_num < thread_pool->max_num_) {
            pthread_mutex_lock(&thread_pool->mutex_pool_);

            int counter = 0;

            for (int i = 0;
                 i < thread_pool->max_num_ && counter < kMaxAppendNum && thread_pool->alive_num_ < thread_pool->max_num_; ++i) {
                if (thread_pool->thread_ids_[i] == 0) {
                    pthread_create(&thread_pool->thread_ids_[i], NULL, worker, thread_pool);

                    counter++;
                    thread_pool->alive_num_++;
                }
            }

            pthread_mutex_unlock(&thread_pool->mutex_pool_);
        }

        if (busy_num * 2 < alive_num && alive_num > thread_pool->min_num_) {
            pthread_mutex_lock(&thread_pool->mutex_pool_);

            thread_pool->exit_num_ = kMaxAppendNum;

            pthread_mutex_unlock(&thread_pool->mutex_pool_);

            for (int i = 0; i < kMaxAppendNum; ++i) {
                pthread_cond_signal(&thread_pool->not_empty_);
            }
        }
    }

    return nullptr;
}

template <class T>
void* ThreadPool<T>::worker(void* arg) {
    ThreadPool<T>* thread_pool = static_cast<ThreadPool<T>*>(arg);

    while (true) {
        pthread_mutex_lock(&thread_pool->mutex_pool_);

        while (thread_pool->task_queue_->tasks_number() == 0 && !thread_pool->shotdown_) {
            pthread_cond_wait(&thread_pool->not_empty_, &thread_pool->mutex_pool_);

            if (thread_pool->exit_num_ > 0) {
                thread_pool->exit_num_--;

                if (thread_pool->alive_num_ > thread_pool->min_num_) {
                    thread_pool->alive_num_--;

                    pthread_mutex_unlock(&thread_pool->mutex_pool_);

                    thread_pool->thread_exit();
                }
            }
        }

        if (thread_pool->shotdown_) {
            pthread_mutex_unlock(&thread_pool->mutex_pool_);

            thread_pool->thread_exit();
        }

        Task<T> task = thread_pool->task_queue_->take_task();

        thread_pool->busy_num_++;

        pthread_mutex_unlock(&thread_pool->mutex_pool_);

        printf("thread %p starts working...\n", pthread_self());

        task.func_(task.arg_);

        delete task.arg_;
        task.arg_ = nullptr;

        printf("thread %p ends working...\n", pthread_self());
        pthread_mutex_lock(&thread_pool->mutex_pool_);

        thread_pool->busy_num_--;

        pthread_mutex_unlock(&thread_pool->mutex_pool_);
    }

    return nullptr;
}

template <class T>
void ThreadPool<T>::thread_exit() {
    pthread_t tid = pthread_self();

    for (int i = 0; i < max_num_; ++i) {
        if (thread_ids_[i] == tid) {
            thread_ids_[i] = 0;

            printf("thread %p exiting...\n", tid);

            break;
        }
    }

    pthread_exit(NULL);
}