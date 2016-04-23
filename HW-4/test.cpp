#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include "spsc_ring_buffer.h"

const int COUNT = 99;

uint64_t producer_work_loop(spsc_ring_buffer<int>& channel) {
	uint64_t sum = 0;
	for (int i = 0; i < COUNT; ++i)	{
		std::cout << "p " << i << std::endl;
		while (!channel.enqueue(i) ) {
			std::this_thread::yield();
		}
		sum += i;	
	}
	return sum;
}

uint64_t consumer_work_loop(spsc_ring_buffer<int>& channel) {
	uint64_t sum = 0;
	for (int i = 0; i < COUNT; ++i) {
		int elem;
		while (!channel.dequeue(elem) ) {
			std::this_thread::yield();
		}
		std::cout << "c " << elem << std::endl;
		sum += elem;
	}
	return sum;
}

int main() {
	spsc_ring_buffer<int> channel(1024);

	std::future<uint64_t> consumed_sum = std::async(consumer_work_loop, std::ref(channel) );
	std::future<uint64_t> produced_sum = std::async(producer_work_loop, std::ref(channel) );

	consumed_sum.wait();
	produced_sum.wait();

	std::cout << "consumed_sum = " << consumed_sum.get() << std::endl;  //не работает
	std::cout << "produced_sum = " << produced_sum.get() << std::endl;

	return 0;
}