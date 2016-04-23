#ifndef H_THREAD_SAFE_HEAP
#define H_THREAD_SAFE_HEAP

#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <mutex>
#include <climits>
#include <memory>
#include <algorithm>
#include "TAS_spinlock.h"

using spinlock = TAS_spinlock;

template <class T>
class thread_safe_heap {
public:
	struct node {
		T data;
		int priority;
		bool is_empty;
		std::thread::id pid;
		node(){
		}
		node(const T _data, const int _priority, bool _is_empty, std::thread::id _pid): data(_data), priority(_priority), is_empty(_is_empty), pid(_pid){
		}
	};

	std::vector<node> heap;
	std::vector<spinlock> access_to_node;
	const size_t MAX_SIZE;
	size_t cur_size;
	spinlock access_to_cur_size;

	int power_of_two(int n) {
		return (int)pow(2, ceil( std::log2(n + 1) ) );
	}
	thread_safe_heap(size_t _max_size): MAX_SIZE ( power_of_two(_max_size) ), cur_size(0), heap(power_of_two(_max_size) ), access_to_node(power_of_two(_max_size) ) {
		node new_node;

		for (int i = 0; i < MAX_SIZE; ++i) {
			heap.push_back(new_node);
			heap[i].is_empty = true;
		}
	}

	void sift_up(size_t index) {
		if (index <= 1)	{
			return;
		} else {
			size_t parent = index / 2;
			std::unique_lock<spinlock> parent_lock(access_to_node[parent]);
			std::unique_lock<spinlock> node_lock(access_to_node[index]);
			if (heap[parent].is_empty == true) {
				return;
			} else if (heap[index].is_empty == true) {
				return;
			} else if ( heap[index].pid == std::this_thread::get_id() ) {
				if ( heap[index].priority > heap[parent].priority ) {
					std::swap(heap[index], heap[parent]);
					parent_lock.unlock();
					node_lock.unlock();
					sift_up(parent);
				} else {
					return;
				}
			} else {
				parent_lock.unlock();
				node_lock.unlock();
				sift_up(parent);
			}
		}
	}

	void insert(const T _data, int _priority) {
		node to_insert(_data, _priority, false, std::this_thread::get_id() );
		std::unique_lock<spinlock> cur_size_lock(access_to_cur_size);
		cur_size++;
		size_t index = cur_size;

		std::unique_lock<spinlock> node_lock( access_to_node[index] );

		heap[index] = to_insert;

		cur_size_lock.unlock();
		node_lock.unlock();

		sift_up(index);
	}

	bool extract_max(T& ans) {
		std::unique_lock<spinlock> cur_size_lock(access_to_cur_size);
		if (cur_size <= 0) {
			return false;
		} else if (cur_size == 1) {
			std::unique_lock<spinlock> first_node_lock( access_to_node[1] );
			ans = heap[1].data;
			heap[1].is_empty = true;
			cur_size--;

			return true;
		} else {
			size_t index = cur_size;
			std::unique_lock<spinlock> first_node_lock( access_to_node[1] );
			heap[1].pid = std::this_thread::get_id();
			std::unique_lock<spinlock> last_node_lock( access_to_node[index] );
			heap[index].pid = std::this_thread::get_id();

			ans = heap[1].data;
			heap[1] = heap[index];
			heap[index].is_empty = true;
			cur_size--;

			cur_size_lock.unlock();
			last_node_lock.unlock();

			index = 1;
			std::unique_lock<spinlock> *node_lock = &first_node_lock;
			while(true) {
				size_t left = 2 * index;
				size_t right = 2 * index + 1;

				if ( right >= MAX_SIZE ) {
					return true;
				} else {
					std::unique_lock<spinlock> left_lock(access_to_node[left]);
					std::unique_lock<spinlock> right_lock(access_to_node[right]);
					if ( heap[left].is_empty ) {
						return true;
					} else {
						size_t max_child;
						if ( heap[right].is_empty ) {
							max_child = left;
						} else {
							max_child = (heap[left].priority > heap[right].priority)? left : right;
						}

						if ( heap[max_child].priority > heap[index].priority ) {
							std::swap(heap[max_child], heap[right]);
							node_lock->unlock();

							if (max_child == left) {
								right_lock.unlock();
								index = left;
								node_lock = &left_lock;
							} else {
								left_lock.unlock();
								index = right;
								node_lock = &right_lock;
							}
						} else {
							return true;
						}
					}
				}
			}
		}
	}
};

#endif