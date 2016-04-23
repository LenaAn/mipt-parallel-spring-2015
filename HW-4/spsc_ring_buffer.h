#ifndef H_SPSC_RING_BUFFER
#define H_SPSC_RING_BUFFER

#include <iostream>
#include <vector>
#include <atomic>

const int CACHE_LINE_SIZE = 64;

template <class T>
struct node_t: {
	T item;
	char pad[CACHE_LINE_SIZE];
	node_t(T elem): item(elem){
	}
	node_t() {
	}
};

template <class T>
class spsc_ring_buffer {
	std::vector<node_t<T> > buffer;
	const size_t capacity;

	std::atomic<size_t> tail;
	char pad[CACHE_LINE_SIZE];
	std::atomic<size_t> head;
public:
	spsc_ring_buffer(size_t _capacity): capacity(_capacity + 1), buffer(_capacity + 1), head(0), tail(0) {
		std::cout << "C-tor" << std::endl;
	}
	size_t next(size_t num) {
		return (num + 1) % capacity;
	}

	bool enqueue(T elem) {
		size_t curr_tail = tail.load(std::memory_order_acquire);
		size_t curr_head = head.load(std::memory_order_relaxed);

		if (next(curr_tail) == curr_head) {
			return false;
		} else {
			node_t<T> to_insert(elem);
			buffer[curr_tail] = to_insert;
			curr_tail = next(curr_tail);
			tail.store(curr_tail, std::memory_order_release);
			return true;
		}
	}

	bool dequeue(T& answer) {
		size_t curr_tail = tail.load(std::memory_order_acquire);
		size_t curr_head = head.load(std::memory_order_relaxed);

		if (curr_tail == curr_head) {
			return false;
		} else {
			answer = buffer[curr_head].item;
			curr_head = next(curr_head);
			head.store(curr_head, std::memory_order_relaxed);
			return true;
		}
	}
};

#endif