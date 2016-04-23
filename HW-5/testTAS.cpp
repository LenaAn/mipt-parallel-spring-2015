#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <chrono>
#include "TAS_spinlock.h"

const int M = 10000000;
int a = 0;
TAS_spinlock TAS_spin;
std::vector<std::thread> threads;

void TAS_incs() {
	for (int i = 0; i < M; ++i)	{
		TAS_spin.lock();
		a++;
		TAS_spin.unlock();
	}
}

int main() {

	for (int n = 1; n < 11; ++n) {
		auto start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < n; ++i) {
			threads.emplace_back(TAS_incs);
		}
		for (int i = 0; i < n; ++i) {
			threads[i].join();
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end-start;
		std::cout << "Time for " << n << " TAS threads : " << diff.count() << " s\n";

		threads.erase( threads.begin(), threads.end() );
	}

	return 0;
}