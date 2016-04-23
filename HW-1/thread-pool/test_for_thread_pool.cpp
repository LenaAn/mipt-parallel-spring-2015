#include <iostream>
#include <fstream>
#include <cmath>
#include <thread>
#include <future>
#include <chrono>

#include "thread_pool.h"

const int NUM_TH = 5;

bool is_prime(size_t n) {
	for (int i = 2; i < ceil(sqrt(n + 1)); ++i)	{
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

std::thread::id return_id() {
	return std::this_thread::get_id();
}

int main() {
	freopen("output", "w", stdout);

	thread_pool<bool> pool(NUM_TH);
	std::future<bool> fut;

	for (int i = 3; i < 10; ++i) {
		pool.submit( std::bind(is_prime, i), fut );
		bool result_value = fut.get();
		if (result_value) {
			std::cout << i << " is prime " << std::endl;
		} else {
			std::cout << i << " is composite " << std::endl;
		}
	}
	pool.shut_down();

	thread_pool<std::thread::id> pool_for_id;
	std::future<std::thread::id> fut_for_id;

	for (int i = 0; i < 10; ++i) {

		pool_for_id.submit(return_id, fut_for_id);
		std::cout << fut_for_id.get() << std::endl;
	}

	pool_for_id.shut_down();
	return 0;
}