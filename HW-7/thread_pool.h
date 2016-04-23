#ifndef H_THREAD_POOL
#define H_THREAD_POOL

#include <functional>
#include <vector>
#include <thread>
#include <future>
#include <iostream>
#include <condition_variable>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <iostream>

#include "thread_safe_queue.h"

const int MAX_SIZE = 10000000;

size_t default_num_workers() {
	if (std::thread::hardware_concurrency() > 0) {
		return std::thread::hardware_concurrency();
	} else {
		return 4;
	}
}

template <class T>
class thread_pool {
	thread_safe_queue<std::packaged_task<T()> > pool;
	size_t number_of_workers;
	std::vector<std::thread> workers;
	std::atomic<bool> flag;

public:
	thread_pool(int _n = default_num_workers() ): number_of_workers(_n), workers(_n), pool(MAX_SIZE) {
		flag.store(false);
		for (int i = 0; i < number_of_workers; ++i) {

			workers[i] = std::thread([i, this](){
						std::packaged_task<T()> new_task;

						while (!flag.load()) {

							if (pool.try_pop(new_task)) {
								new_task();
							} else {
								std::this_thread::yield();
							}
						}

					}
			);
		}
	}
	void shut_down() {
		flag.store(true);
	}
	~thread_pool() {
		for_each (workers.begin(), workers.end(), [](std::thread &t) 
    												{
        												t.join();
   													});
	}

	bool submit(std::function<T()> func, std::future<T>& fut) {
		std::packaged_task<T()> new_task(func);
		fut = new_task.get_future();

		if (!pool.is_full()) {
			pool.push(std::move(new_task));
			return true;
		} else {
			return false;
		}
	}

	void active_wait(std::future<T>& fut) {
		while (fut.wait_for(std::chrono::seconds(0) ) != std::future_status::ready ) {
			std::packaged_task<T()> new_task;
			if (pool.try_pop(new_task) ) {
				new_task();
			} else {
				std::this_thread::yield();
			}
		}
	}
};

#endif