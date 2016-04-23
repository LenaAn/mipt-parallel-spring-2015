#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <chrono>
#include "TATAS_spinlock.h"

const int M = 10000000;
int a = 0;
TATAS_spinlock TATAS_spin;
std::vector<std::thread> threads;

void TATAS_incs() {
	for (int i = 0; i < M; ++i)	{
		TATAS_spin.lock();
		a++;
		TATAS_spin.unlock();
	}
}

int main() {

	for (int n = 1; n < 11; ++n) {
		auto start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			threads.emplace_back(TATAS_incs);
		}
		for (int i = 0; i < n; ++i) {
			threads[i].join();
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end-start;
		std::cout << "Time for " << n << " TATAS threads : " << diff.count() << " s\n";

		threads.erase( threads.begin(), threads.end() );
	}

	return 0;
}