#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "task_queue.cpp"
#include "task_queue.h"

template <class T>
class ThreadPool {
  public:
    ThreadPool(int min_num_, int max_num);

    ~ThreadPool();

    void add_task(Task<T> task);

    int get_busy_num();

    int get_alive_num();

  private:
    static void* manager(void* arg);

    static void* worker(void* arg);

    void thread_exit();

    TaskQueue<T>* task_queue_;

    pthread_t manager_id_;
    pthread_t* thread_ids_;

    int min_num_;
    int max_num_;
    int busy_num_;
    int alive_num_;
    int exit_num_;

    static const int kMaxAppendNum = 2;

    pthread_mutex_t mutex_pool_;

    pthread_cond_t not_empty_;

    bool shotdown_;
};

#endif  // THREAD_POOL_H