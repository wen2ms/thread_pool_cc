#include "task_queue.h"

TaskQueue::TaskQueue() {
    pthread_mutex_init(&mutex_, NULL);
}

TaskQueue::~TaskQueue() {
    pthread_mutex_destroy(&mutex_);
}

void TaskQueue::add_task(Task task) {
    pthread_mutex_lock(&mutex_);

    task_queue_.push(task);

    pthread_mutex_unlock(&mutex_);
}

void TaskQueue::add_task(callback_func func, void* arg) {
    pthread_mutex_lock(&mutex_);

    task_queue_.push(Task(func, arg));

    pthread_mutex_unlock(&mutex_);
}

Task TaskQueue::take_task() {
    Task task;

    pthread_mutex_lock(&mutex_);
    if (!task_queue_.empty()) {
        task = task_queue_.front();
        task_queue_.pop();
    }

    pthread_mutex_unlock(&mutex_);

    return task;
}