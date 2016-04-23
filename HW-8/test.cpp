#include <iostream>
#include <vector>
#include <random>

#include "parallel_scan.h"

using int_t = uint64_t;

const int N = 102;

template <class T>
class add {
public:
	add(){};
	T operator()(T& first, T& second) {
		return first + second;
	}
};

int main() {
	std::random_device random_dev;
	std::mt19937 mt_random_engine(random_dev());
	std::uniform_int_distribution<int_t> gen_random_int(1, 1000);

	std::vector<int> data;
	for (int i = 0; i < N; ++i)	{
		data.push_back( gen_random_int(mt_random_engine) );
	}

	std::cout << "data:" << std::endl;
	for (int i = 0; i < N; ++i) {
		std::cout << data[i] << ' ';
	}
	std::cout << std::endl << std::endl;

	parallel_scan(data, add<int>(), 4);

	std::cout << "prefix_sums:" << std::endl;
	for (int i = 0; i < N; ++i)	{
		std::cout << data[i] << ' ';
	}
	std::cout << std::endl;
}