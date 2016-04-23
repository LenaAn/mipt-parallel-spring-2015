#ifndef H_HIERARCHICAL_MUTEX
#define H_HIERARCHICAL_MUTEX

#include <cassert>
#include <thread>
#include <stack>
#include <mutex>
#include <exception>

thread_local std::stack<size_t> prev_levels;

class hierarchical_mutex {
	std::mutex mtx;
	size_t level;

public:
	hierarchical_mutex(size_t _level): level(_level) {
	};
	void lock() {
		if (prev_levels.empty() || prev_levels.top() > level) {
			mtx.lock();
			prev_levels.push(level);
			return;
		} else {
			throw std::logic_error("hierarchy violation");
		}
	};
	void unlock() {
		if (level == prev_levels.top()) {
			mtx.unlock();
			prev_levels.pop();
			return;
		} else {
			throw std::logic_error("hierarchy violation");
		}
	};
	~hierarchical_mutex() {
		assert( mtx.try_lock() );
		mtx.unlock();
	};
};

#endif