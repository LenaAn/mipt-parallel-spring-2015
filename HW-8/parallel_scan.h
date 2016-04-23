#ifndef H_PARALLEL_SCAN
#define H_PARALLEL_SCAN

#include <atomic>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

#include "cyclic_barrier.h"

template <typename T, class Add>
void parallel_scan(std::vector<T>& data, Add add, size_t num_threads) {
	size_t n = data.size();

	cyclic_barrier barrier(num_threads);

	std::vector<std::thread> workers;	//завести массив потоков
	std::vector<T> prefix_of_chuncks(num_threads, 0);

	for (int i = 0; i < num_threads; ++i) {
		auto worker = [&](size_t i) {
			size_t first = i * (n / num_threads);	//first included		//определить свою зону ответственности
			size_t last;
			if (i == num_threads - 1) {
				last = n;
			} else {
				last = first + (n / num_threads);
			}
			
			for (int j = first + 1; j < last; ++j) {
				data[j] += data[j - 1];
			}

			barrier.enter();

			if (i == 0) {
				for (int j = 1; j < num_threads; ++j) {
					int right = j * (n / num_threads) - 1;
					prefix_of_chuncks[j] = prefix_of_chuncks[j - 1] + data[right];
				}
			}

			barrier.enter();

			if (i == 0)	{
				return;
			} else {
				for (int j = first; j < last; ++j) {
					data[j] += prefix_of_chuncks[i];
				}
			}
		};
		workers.emplace_back(worker, i);	//и запустить поток с функцией
	}

	for (auto& t: workers) {
		t.join();
	}
}

#endif