#include <unistd.h>

#include <iostream>

#include "thread_pool.cpp"
#include "thread_pool.h"

void task_func(void* arg) {
    int* num = static_cast<int*>(arg);

    std::cout << "thread " << static_cast<void*>(pthread_self()) << ", number = " << *num << std::endl;

    sleep(1);
}

int main() {
    ThreadPool<int> thread_pool(3, 10);

    for (int i = 0; i < 100; ++i) {
        int* num = new int(100 + i);

        thread_pool.add_task(Task<int>(task_func, num));
    }

    sleep(30);

    return 0;
}