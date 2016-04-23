#include <iostream>
#include <vector>

#include "fft.h"

const int N = 160;

int main() {
	std::vector<int> a, b, res;
	for (int i = 0; i < N; ++i)	{
		a.push_back(i);
		b.push_back(i + N);
	}

	parallel_multiply(a, b, res);

	for (int i = 0; i < 2 * N; ++i)	{
		std::cout << res[i] << ' ';
	}
	std::cout << std::endl;

}