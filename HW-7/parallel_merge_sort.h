#ifndef H_PARALLEL_MERGE_SORT
#define H_PARALLEL_MERGE_SORT

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include "thread_pool.h"

const int MAX_REC_DEPTH = 10;

template <class T>
class sort_task {
	std::vector<T> &data;
	size_t recursion_depth;
	size_t begin;
	size_t end;
	thread_pool<bool> *ptr_pool;
public:
	sort_task(std::vector<T> &_data, size_t _begin, size_t _end, thread_pool<bool> *_ptr_pool, size_t rec_depth): data(_data), begin(_begin), end(_end), ptr_pool(_ptr_pool), recursion_depth(rec_depth) {
	}
	bool should_switch_to_sequential_sort(size_t rec_depth) {
		return rec_depth > MAX_REC_DEPTH;
	}
	bool operator() () {
		std::mutex mtx;
		if (should_switch_to_sequential_sort(recursion_depth) ) {
			std::sort(data.begin() + begin, data.begin() + end);
		} else {
			sort_task sort_left_part(data, begin, (end + begin) / 2, ptr_pool, recursion_depth + 1);
			sort_task sort_right_part(data, (end + begin) / 2, end, ptr_pool, recursion_depth + 1);

			std::future<bool> left_part_sorted;

			while (!ptr_pool->submit(sort_left_part, left_part_sorted) == true) {
				std::this_thread::yield();
			}
			sort_right_part();
			ptr_pool->active_wait(left_part_sorted);
			std::inplace_merge(data.begin() + begin, data.begin() + (end + begin) / 2, data.begin() + end );
		}
		return true;
	}
};

template <class T>
void parallel_merge_sort(std::vector<T> &data, int begin, int end) {
	thread_pool<bool> pool;

	std::future<bool> arr_sorted;
	pool.submit(sort_task<int>(data, begin, end, &pool, 0), arr_sorted );

	arr_sorted.get();
	pool.shut_down();
	return;
}

#endif