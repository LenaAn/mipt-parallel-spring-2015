#include <iostream> 
#include <thread>
#include <cstdlib>
#include <mutex>
#include <vector>
#include <queue>
#include <ctime>
#include <cmath>
#include <condition_variable>
#include <random>
#include "thread_safe_queue.h"

const std::size_t NUMBER_OF_QUERIES = 100;
const std::size_t NUMBER_OF_CONSUMERS = 10;
const std::size_t END_MARKER = -1;

bool is_prime(const int n) {
	for (int i = 2; i * i <= n; ++i) {
		if (n % i == 0)	{
			return false;
		}
	}
	return true;
}

void produce(const int n, thread_safe_queue<int>& q, std::mutex& output_access) {
    std::random_device rd;
    std::default_random_engine rand_en(rd());
    std::uniform_int_distribution<int> uniform_dist(1, 10000);

	for (int i = 0; i < n; ++i) {
		const int new_elem = uniform_dist(rand_en);
		q.push(new_elem);
		std::lock_guard<std::mutex> output_lock(output_access);
		std::cout << "I've written " << new_elem << std::endl;
	}
	//shuting down
	q.push(END_MARKER);
}

void consume(thread_safe_queue<int>& q, std::mutex& output_access) {
	while (true) {
		int next;
		q.pop(next);
		if (next == END_MARKER) {
			q.push(END_MARKER);
			break;
		} else {
			if (is_prime(next))	{
				std::lock_guard<std::mutex> output_lock(output_access);
				std::cout << next << " is prime" << std::endl;
			} else {
				std::lock_guard<std::mutex> output_lock(output_access);
				std::cout << next << " is composite" << std::endl;		
			}
		}
	}
}

int main() {
	thread_safe_queue<int> q;
	std::mutex output_access;

	std::thread producer(produce, NUMBER_OF_QUERIES, std::ref(q), std::ref(output_access));

	std::vector<std::thread> consumers;
	for (int i = 0; i < NUMBER_OF_CONSUMERS; ++i) {
		consumers.emplace_back(consume, std::ref(q), std::ref(output_access));
	}

	producer.join();
	for (auto& t: consumers) {
		t.join();
	}

	printf("It worked!\n");
	return 0;
}
