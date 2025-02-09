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

        pthread_create(&manager_id_, nullptr, manager, thread_pool);
        for (int i = 0; i < min_num; ++i) {
            pthread_create(&thread_pool->thread_ids[i], NULL, worker, thread_pool);
        }

        return;
    } while (0);

    if (thread_ids_) {
        delete[] thread_ids_;
    }

    if (task_queue_) {
        delete task_queue_;
    }
}