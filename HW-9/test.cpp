#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "parallel_gauss.h"
#include "timer.h"

using v_double = std::vector<double>;

void scan(int n, v_double& vect) {
	vect.resize(n);
	for (int i = 0; i < n; ++i)	{
		std::cin >> vect[i];
	}
}

void print(v_double& answer) {
	int n = answer.size();
	for (int i = 0; i < n; ++i)	{
		printf("%14.8lf ", answer[i]);
	}
	printf("\n");
}

int main() {
	freopen("output.txt", "w", stdout);
	freopen("input.txt", "r", stdin);

	int n;
	std::cin >> n;

	std::vector<v_double> matrix(n);
	v_double b, answer;

	for (int i = 0; i < n; ++i)	{
		scan(n, matrix[i]);
	}
	scan(n, b);

    steady_timer timer;

    bool result = parallel_gauss(n, matrix, b, answer);

    const double work_time = timer.seconds_elapsed();

	if (result) {
		print(answer);
	} else {
		std::cout << "no solution" << std::endl;
	}
	printf("work time = %.5lf\n", work_time);
	return 0;
}