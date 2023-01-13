/*
 * Created by Max Azimi on 3/22/2022 AD.
 */

#ifndef SCHEDULER_C_H
#define SCHEDULER_C_H

#include <queue>
#include <mutex>

class Task // base Task class
{
public:
	pthread_mutex_t mLock{};

	Task() : mCondition(true) {}
	virtual ~Task() = default;

	virtual void run() = 0; // must be thread-safe
	inline void terminate() { mCondition = false; }

protected:
	bool mCondition;
};

typedef struct
{
	std::deque<Task *> *taskQueue;
	pthread_mutex_t *queueLock;
	pthread_cond_t *queueCond;
	volatile unsigned int *tasksRunning;
	pthread_cond_t *tasksFinishedCond;
} thread_args_t;

class Scheduler
{
public:
	Scheduler();
	~Scheduler();

	void schedule(Task *task);
	void waitUntilFinished();

private:
	std::vector<pthread_t> mThreads;
	std::deque<Task *> mTaskQueue;
	pthread_mutex_t mQueueLock;
	pthread_cond_t mQueueCond;

	thread_args_t mThreadArgs;

	pthread_cond_t mTasksFinishedCond;
	volatile unsigned int mTasksRunning; // protected by queue_lock
};

void *thread_function(void *args);

#endif // SCHEDULER_C_H
