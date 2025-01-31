#ifndef TASK_QUEUE
#define TASK_QUEUE

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

    Task take_task();
    Task take_task(callback_func func, void* arg);

    inline int tasks_number() {
        return task_queue_.size();
    }

  private:
    pthread_mutex_t mutex_;
    std::queue<Task> task_queue_;
};

#endif  // TASK_QUEUE