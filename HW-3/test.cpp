#include <iostream>
#include <thread>
#include "TAS_spinlock.h"
#include "thread_safe_heap.h"

const int SIZE = 20;
TAS_spinlock access_to_output;

void producer_1(thread_safe_heap<int>& heap) {
	for (int i = 0; i < SIZE / 2; ++i) {
		heap.insert(i, i);
		access_to_output.lock();
		std::cout << "producer 1 inserted " << i << std::endl;
		access_to_output.unlock();
	}
}
void producer_2(thread_safe_heap<int>& heap) {
	for (int i = SIZE / 2; i < SIZE; ++i) {
		heap.insert(i, i);
		access_to_output.lock();
		std::cout << "producer 2 inserted " << i << std::endl;
		access_to_output.unlock();
	}
}
void consumer(thread_safe_heap<int>& heap, int n) {
	int el;
	while (heap.extract_max(el)) {
		access_to_output.lock();
		std::cout << "consumer " << n << " extracted " << el << std::endl;
		access_to_output.unlock();
		while (heap.extract_max(el)) {
			access_to_output.lock();
			std::cout << "consumer " << n << " extracted " << el << std::endl;
			access_to_output.unlock();
		}
		std::this_thread::yield();
	}
}

int main() {
	thread_safe_heap<int> heap(SIZE);
	std::thread pr1(producer_1, std::ref(heap));
	std::thread pr2(producer_2, std::ref(heap));
	std::thread cn1(consumer, std::ref(heap), 1);
	std::thread cn2(consumer, std::ref(heap), 2);

	pr1.join();
	pr2.join();
	cn1.join();
	cn2.join();

	return 0;
}