#ifndef H_FFT
#define H_FFT

#include <cassert>
#include <cmath>
#include <complex>
#include <iostream>
#include <thread>
#include <vector>

#include "cyclic_barrier.h"

const int DEFAULT_NUM = 4;
typedef std::complex<double> base;
 
int rev (int num, int lg_n) {
	int res = 0;
	for (int i = 0; i < lg_n; ++i)
		if ( num & (1 << i) )
			res |= 1 << (lg_n - 1 - i);
	return res;
}

void reverse_block(std::vector<base>& a, int begin, int end) {
	int n = (int) a.size();
	int lg_n = 0;
	while ((1 << lg_n) < n)  ++lg_n;

	for (int i = begin; i < end; ++i)
		if (i < rev(i, lg_n))
			swap (a[i], a[ rev(i, lg_n) ]);
}

void fft_bottom_up_by_block (std::vector<base>& a, bool invert, int begin, int end) {		//begin included, end excluded
	int n = end - begin;

	for (int len = 2; len <= n; len <<= 1) {		//по длине субблока
		double ang = 2 * M_PI / len * (invert ? -1 : 1);
		base wlen (cos(ang), sin(ang));
		for (int i = begin; i < end; i += len) {		//перемещаемся между субблоками
			base w (1);
			for (int j = 0; j < len / 2; ++j) {		//идем по субблоку
				base u = a[i + j],  v = a[i + j + len / 2] * w;
				a[i + j] = u + v;
				a[i + j + len / 2] = u - v;
				w *= wlen;
			}
		}
	}
}

void fft_bottom_up_in_cycle(std::vector<base>& a, bool invert, int number, int p) {
	int n = (int) a.size();

	for (int len = 2 * n / p; len <= n; len <<= 1) {		//по длине субблока
		double ang = 2 * M_PI / len * (invert ? -1 : 1);
		for (int i = 0; i < n; i += len) {		//перемещаемся между субблоками
			base w ( cos(number * ang), sin(number * ang) );

			for (int j = number; j < len / 2; j += p ) {		//в каждом субблоке делаем только свою работу
				base u = a[i + j],  v = a[i + j + len / 2] * w;
				a[i + j] = u + v;
				a[i + j + len / 2] = u - v;

				base w_fact( cos(p * ang), sin(p * ang) );
				w *= w_fact;
			}
		}
	}
}
size_t number_of_threads(size_t n) {
	int hardware = std::max((int)std::thread::hardware_concurrency(), DEFAULT_NUM);
	size_t p = 1;
	while (2 * p <= hardware && 4 * p * p <= n) {
		p *= 2;
	}
	return p;
}

void parallel_multiply(const std::vector<int> & a, const std::vector<int> & b, std::vector<int> & res) {
	std::vector<base> fa (a.begin(), a.end()),  fb (b.begin(), b.end());
	size_t n = 1;
	while (n < std::max (a.size(), b.size()))  n <<= 1;
	n <<= 1;
	std::cout << "n = " << n << std::endl;
	fa.resize (n),  fb.resize (n), res.resize(n);
	int p = number_of_threads(n);

	std::cout << "p = " << p << std::endl;
	cyclic_barrier barrier(p);

	auto work = [&](int i) {
		int begin = i * (n / p);
		int end = (i + 1) * (n / p);
		reverse_block(fa, begin, end), reverse_block(fb, begin, end);
		barrier.enter();
		fft_bottom_up_by_block(fa, false, begin, end), fft_bottom_up_by_block(fb, false, begin, end);
		barrier.enter();	//1
		fft_bottom_up_in_cycle(fa, false, i, p), fft_bottom_up_in_cycle(fb, false, i, p);

		for (int j = i; j < n; j += p) {
			fa[j] *= fb[j];
		}
		barrier.enter();

		reverse_block(fa, begin, end);
		barrier.enter();

		for (int j = begin; j < end; ++j) {
			fa[j] /= n;
		}
		fft_bottom_up_by_block(fa, true, begin, end);
		barrier.enter();
		fft_bottom_up_in_cycle(fa, true, i, p);
	
		for (int j = i; j < n; j += p)	{
			res[j] = (int) (fa[j].real() + 0.5);
		}
	};
	std::vector<std::thread> threads;
	for (int i = 0; i < p; ++i) {
		threads.emplace_back(work, i);
	}
	for (auto& t: threads) {
		t.join();
	}
}

#endif