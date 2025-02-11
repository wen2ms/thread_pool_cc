#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <pthread.h>

using callback_func = void (*)(void* arg);

class Task {
  public:
    Task() : func_(nullptr), arg_(nullptr) {}
    
    Task(callback_func func, void* arg) : func_(func), arg_(arg) {}

    callback_func func_;
    void* arg_;
};

class TaskQueue {
  public:
    TaskQueue();
    ~TaskQueue();

    void add_task(Task task);
    void add_task(callback_func func, void* arg);

    Task take_task();

    inline size_t tasks_number() {
        return task_queue_.size();
    }

  private:
    pthread_mutex_t mutex_;
    std::queue<Task> task_queue_;
};

#endif  // TASK_QUEUE_H