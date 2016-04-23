#ifndef H_PARALLEL_GAUSS
#define H_PARALLEL_GAUSS

#include <atomic>
#include <cassert>
#include <cmath>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "cyclic_barrier.h"

using v_double = std::vector<double>;

const double EPS = 1e-5;
const size_t NUM_OF_THREADS = 4;

inline size_t find_max_row(size_t number, size_t n, size_t start, std::vector<v_double>& matrix) {	//если ни одна строка из зоны 
	int i = (start / NUM_OF_THREADS) * NUM_OF_THREADS + number;
	if (i < start) {
		i += NUM_OF_THREADS;
	}																//значение >=n
	size_t answer = i;
	for (; i < n; i = i + NUM_OF_THREADS) {
		if ( fabs(matrix[i][start]) > fabs(matrix[answer][start]) ) {
			answer = i;
		}
	}
	return answer;
}

inline void compute_matrix(size_t number, size_t n, size_t start, std::vector<v_double>& matrix, v_double& b) {
	int i = (start / NUM_OF_THREADS) * NUM_OF_THREADS + number;
	if (i <= start) {
		i += NUM_OF_THREADS;
	}

	for (; i < n; i += NUM_OF_THREADS) {
		assert( fabs( matrix[start][start] ) > EPS );

		double koef = matrix[i][start] / matrix[start][start];		//нашли коэффициент
		for (int j = start; j < n; ++j) {
			matrix[i][j] -= koef * matrix[start][j];		//пересчитали ниже стоящие строки
		}
		b[i] -= koef * b[start];
	}
}

inline void substruct_from_b(size_t number, size_t n, std::vector<v_double>& matrix, size_t step, v_double& b, v_double& answer) {
	assert(answer[step] != 0);
	for (int i = number; i < step; i += NUM_OF_THREADS) {
		b[i] -= matrix[i][step] * answer[step];
	}
}

bool parallel_gauss(size_t n, std::vector<v_double>& matrix, v_double& b, v_double& answer) {
	answer.resize(n);

	std::vector<std::thread> threads(NUM_OF_THREADS);
	std::vector<size_t> index_of_max(NUM_OF_THREADS);		//индекс кандидата на pivot_row от зоны ответсвенности этого потока
	cyclic_barrier barrier_for_n_plus_1(NUM_OF_THREADS + 1);		//для синхронизации при поиске pivot и back_substitution
	cyclic_barrier barrier_for_n(NUM_OF_THREADS);	

	auto worker = [&](size_t number) {		//number == номер потока
		for (size_t i = 0; i < n; ++i)	{			//gauss elimination по строчкам
			index_of_max[number] = find_max_row(number, n, i, matrix);		//найти мин из своих строк; i == start
			barrier_for_n_plus_1.enter();									//чтобы потом нашли pivot
			///////заменить на cond_var

			compute_matrix(number, n, i, matrix, b);				//пересчитать нижние строчки матрицы, i == start
			barrier_for_n.enter();
		}

		for (int i = n - 1; i >= 0; --i)	{		//back_substitution
			barrier_for_n_plus_1.enter();
			////////////con_var
			assert(answer[i] != 0);
			substruct_from_b(number, n, matrix, i, b, answer);		//i == step;
			
		}
		return;
	};

	for (size_t i = 0; i < NUM_OF_THREADS; ++i) {		//запустить потоки
		threads[i] = std::thread(worker, i);
	}

	for (int i = 0; i < n; ++i) {			//gaussian elimination
		barrier_for_n_plus_1.enter();

		int j = 0;
		while (index_of_max[j] >= n) {
			++j;
		}
		size_t pivot = index_of_max[j];			//и найти pivot из тех, что действительно существуют
		for (; j < NUM_OF_THREADS; ++j) {
			if (index_of_max[j] < n && fabs(matrix[ index_of_max[j] ][i]) > fabs(matrix[pivot][i]) ) {
				pivot = index_of_max[j];
			}
		}

		if ( fabs( matrix[pivot][i] ) < EPS ) {
			for (auto& t: threads) {
				t.detach();
			}
			return false;
		}

		if (pivot != i)	{		//поставить pivot строку на нужное место
			for (int j = 0; j < n; ++j)	{
				std::swap(matrix[pivot][j], matrix[i][j]);
			}
			std::swap(b[pivot], b[i]);
		}	

		////////////////заменить на cond_var
	}
	for (int i = n - 1; i >= 0; --i) {		//back_substitution
		assert(matrix[i][i] != 0);
		barrier_for_n_plus_1.enter();		//жду, пока потоки изменят b
		answer[i] = b[i] / matrix[i][i];
		///////////заменить на cond_var
	}

	for (auto& t: threads) {
		t.join();
	}
	return true;
}

#endif