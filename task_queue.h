#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <pthread.h>

using callback_func = void (*)(void* arg);

template <class T>
class Task {
  public:
    Task<T>() : func_(nullptr), arg_(nullptr) {}
    
    Task<T>(callback_func func, void* arg) : func_(func), arg_(static_cast<T*>(arg)) {}

    callback_func func_;
    T* arg_;
};

template <class T>
class TaskQueue {
  public:
    TaskQueue();
    ~TaskQueue();

    void add_task(Task<T> task);
    void add_task(callback_func func, void* arg);

    Task<T> take_task();

    inline size_t tasks_number() {
        return task_queue_.size();
    }

  private:
    pthread_mutex_t mutex_;
    std::queue<Task<T>> task_queue_;
};

#endif  // TASK_QUEUE_H