#ifndef TASK_QUEUE
#define TASK_QUEUE

using callback_func = void (*)(void* arg);

class Task {
  public:
    Task() : func_(nullptr), arg_(nullptr) {}
    
    Task(callback_func func, void* arg) : func_(func), arg_(arg) {}

    callback_func func_;
    void* arg_;
};

#endif  // TASK_QUEUE