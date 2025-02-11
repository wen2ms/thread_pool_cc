#include <iostream>

#include <unistd.h>

#include "thread_pool.h"

void task_func(void* arg) {
    int* num = static_cast<int*>(arg);

    std::cout << "thread " << pthread_self() << ", number = " << *num << std::endl;

    sleep(1);
}

int main() {
    ThreadPool thread_pool(3, 10);

    for (int i = 0; i < 100; ++i) {
        int* num = new int(100 + i);

        thread_pool.add_task(Task(task_func, num));
    }

    sleep(30);

    return 0;
}