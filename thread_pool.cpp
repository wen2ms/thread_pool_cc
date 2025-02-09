#include "thread_pool.h"

#include <string.h>

#include <iostream>

ThreadPool::ThreadPool(int min_num, int max_num) {
    do {
        task_queue_ = new TaskQueue;

        if (task_queue_ == nullptr) {
            std::cout << "new task_queue_ failed..." << std::endl;
            break;
        }

        thread_ids_ = new pthread_t[max_num];

        if (thread_ids_ == nullptr) {
            std::cout << "new thread_ids failed..." << std::endl;
            break;
        }

        memset(thread_ids_, 0, sizeof(pthread_t) * max_num);

        min_num_ = min_num;
        max_num_ = max_num;
        busy_num_ = 0;
        alive_num_ = min_num;
        exit_num_ = 0;

        if (pthread_mutex_init(&mutex_pool_, nullptr) != 0 || pthread_cond_init(&not_empty_, nullptr) != 0) {
            std::cout << "mutex or condition init failed..." << std::endl;
            break;
        }

        shotdown_ = false;

        pthread_create(&manager_id_, nullptr, manager, this);
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

void* ThreadPool::worker(void* arg) {
    ThreadPool* thread_pool = static_cast<ThreadPool*>(arg);

    while (true) {
        pthread_mutex_lock(&thread_pool->mutex_pool_);

        while (thread_pool->task_queue_->tasks_number() == 0 && !thread_pool->shotdown_) {
            pthread_cond_wait(&thread_pool->not_empty_, &thread_pool->mutex_pool_);

            if (thread_pool->exit_num_ > 0)  {
                thread_pool->exit_num_--;

                if (thread_pool->alive_num_ > thread_pool->min_num_) {
                    thread_pool->alive_num_--;

                    pthread_mutex_unlock(&thread_pool->mutex_pool_);

                    thread_exit(thread_pool);
                }
            }
        }

        if (thread_pool->shotdown_) {
            pthread_mutex_unlock(&thread_pool->mutex_pool_);

            thread_exit(thread_pool);
        }

        Task task = thread_pool->task_queue_->take_task();

        thread_pool->busy_num_++;

        pthread_mutex_unlock(&thread_pool->mutex_pool_);

        std::cout << "thread " << pthread_self() << " starts working..." << std::endl;

        task.func_(task.arg_);

        delete task.arg_;
        task.arg_ = nullptr;

        std::cout << "thread " << pthread_self() << " ends working..." << std::endl;
        pthread_mutex_lock(&thread_pool->mutex_pool_);

        thread_pool->busy_num_--;

        pthread_mutex_unlock(&thread_pool->mutex_pool_);
    }

    return nullptr;
}