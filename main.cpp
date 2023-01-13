#include <mutex>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include "scheduler.h"

class MyTask : public Task
{
public:
	MyTask() = default;
	~MyTask() override = default;

	void run() override
	{
		printf("It's MyTask!\n");
	}
};

int main(int argc, char *argv[])
{
	Scheduler scheduler;
	scheduler.schedule(new MyTask());
	scheduler.schedule(new MyTask());
	scheduler.schedule(new MyTask());

#if 1
    int counter = 0;
    std::mutex counter_mutex;
    std::vector<std::thread> threads;

    auto worker_task = [&](int id) {
        std::unique_lock<std::mutex> lock(counter_mutex);
        ++counter;
        std::cout << id << ", initial counter: " << counter << '\n';
        lock.unlock();

        // don't hold the lock while we simulate an expensive operation
        std::this_thread::sleep_for(std::chrono::seconds(1));

        lock.lock();
        ++counter;
        std::cout << id << ", final counter: " << counter << '\n';
    };

    for (int i = 0; i < 10; ++i) threads.emplace_back(worker_task, i);
    for (auto &thread : threads) thread.join();
#endif

	printf("main() ended.\n");
}
