#ifndef H_THREAD_SAFE_QUEUE
#define H_THREAD_SAFE_QUEUE

#include <mutex>
#include <queue>
#include <condition_variable>
#include <iostream>

template <class T>
class thread_safe_queue {
	std::mutex queue_access;
	std::queue<T> q; 
	std::condition_variable not_empty_q;
	size_t MAX_SIZE;
public:
	thread_safe_queue(size_t _max_size): MAX_SIZE(_max_size){
	}
	bool is_full(){
		return (q.size() >= MAX_SIZE);
	}

	void push(T elem) {
		std::unique_lock<std::mutex> lck(queue_access);
		q.push(std::move(elem));
		lck.unlock();
		not_empty_q.notify_one();
	}

	void pop(T& result) {
		std::unique_lock<std::mutex> lck(queue_access);

		while (q.empty()) {
			not_empty_q.wait(lck);   //обязательно еще раз надо проверить
		}

		result = std::move(q.front());   //сделано для thread pool, promise там не копируется
		q.pop();
	}

	bool try_pop(T& result) {
		std::unique_lock<std::mutex> lck(queue_access);

		if (q.empty()) {
			return false;
		} else {
			result = std::move(q.front());   //сделано для thread pool, promise там не копируется
			q.pop();
			return true;
		}
	}
};

#endif