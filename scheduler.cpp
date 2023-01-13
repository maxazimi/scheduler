/*
 * Created by Max Azimi on 3/22/2022 AD.
 */

#include "scheduler.h"

/*
 * TaskScheduler implementation.
 */
Scheduler::Scheduler() :
				mThreadArgs({nullptr}),
				mTasksRunning(0)
{
	pthread_mutex_init(&mQueueLock, nullptr);
	pthread_cond_init(&mQueueCond, nullptr);
	pthread_cond_init(&mTasksFinishedCond, nullptr);

	mThreadArgs.taskQueue = &mTaskQueue;
	mThreadArgs.queueCond = &mQueueCond;
	mThreadArgs.queueLock = &mQueueLock;
	mThreadArgs.tasksRunning = &mTasksRunning;
	mThreadArgs.tasksFinishedCond = &mTasksFinishedCond;
}

Scheduler::~Scheduler()
{
	//for (auto & thread : mThreads) pthread_cancel(thread);
	for (auto & task : mTaskQueue)
		task->terminate();

	waitUntilFinished();
	pthread_cond_broadcast(&mQueueCond);

	for (auto & thread : mThreads)
		pthread_join(thread, nullptr);

	printf("~TaskScheduler()--> number of task(s) terminated: (%lu)\n", mThreads.size());
	mThreads.clear();

	pthread_mutex_destroy(&mQueueLock);
	pthread_cond_destroy(&mQueueCond);
	pthread_cond_destroy(&mTasksFinishedCond);
}

void Scheduler::schedule(Task *task)
{
	if (task == nullptr)
		return;

	pthread_t newThread;
	if (pthread_create(&newThread, nullptr, thread_function, (void *)&mThreadArgs) < 0)
	{
		perror("pthread_create failed");
		newThread = nullptr;
		return;
	}

	mThreads.push_back(newThread);

	pthread_mutex_lock(&mQueueLock);
	mTaskQueue.push_back(task);

	pthread_cond_signal(&mQueueCond);
	pthread_mutex_unlock(&mQueueLock);
}

void Scheduler::waitUntilFinished()
{
	pthread_mutex_lock(&mQueueLock);

	while (!mTaskQueue.empty() || mTasksRunning > 0)
	{
		pthread_cond_wait(&mTasksFinishedCond, &mQueueLock);
	}

	pthread_mutex_unlock(&mQueueLock);
}

void *thread_function(void *args)
{
	auto *threadArgs = (thread_args_t *)args;

	while (pthread_mutex_lock(threadArgs->queueLock) < 0)
	{
		perror("pthread_mutex_lock failed");
	}

	while (threadArgs->taskQueue->empty())
	{
		pthread_cond_wait(threadArgs->queueCond, threadArgs->queueLock);
	}

	auto task = threadArgs->taskQueue->front();   // get pointer to the next task
	threadArgs->taskQueue->pop_front(); // remove it from queue
	(*threadArgs->tasksRunning)++;

	pthread_mutex_unlock(threadArgs->queueLock);

	pthread_mutex_lock(&task->mLock);
	task->run();
	pthread_mutex_unlock(&task->mLock);
	delete task;

	pthread_mutex_lock(threadArgs->queueLock);

	if (--(*threadArgs->tasksRunning) == 0)
	{
		pthread_cond_broadcast(threadArgs->tasksFinishedCond);
	}

	pthread_mutex_unlock(threadArgs->queueLock);
	pthread_exit(nullptr);
}
