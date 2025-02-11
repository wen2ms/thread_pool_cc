#include "task_queue.h"

template <class T>
TaskQueue<T>::TaskQueue() {
    pthread_mutex_init(&mutex_, NULL);
}

template <class T>
TaskQueue<T>::~TaskQueue() {
    pthread_mutex_destroy(&mutex_);
}

template <class T>
void TaskQueue<T>::add_task(Task<T> task) {
    pthread_mutex_lock(&mutex_);

    task_queue_.push(task);

    pthread_mutex_unlock(&mutex_);
}

template <class T>
void TaskQueue<T>::add_task(callback_func func, void* arg) {
    pthread_mutex_lock(&mutex_);

    task_queue_.push(Task<T>(func, arg));

    pthread_mutex_unlock(&mutex_);
}

template <class T>
Task<T> TaskQueue<T>::take_task() {
    Task<T> task;

    pthread_mutex_lock(&mutex_);
    if (!task_queue_.empty()) {
        task = task_queue_.front();
        task_queue_.pop();
    }

    pthread_mutex_unlock(&mutex_);

    return task;
}