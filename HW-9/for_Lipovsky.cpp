#include <iostream>
#include <vector>

using v_double = std::vector<double>;

void scan(int n, v_double& vect) {
	for (int i = 0; i < n; ++i)	{
		double temp;
		std::cin >> temp;
		vect.push_back(temp);
	}
}

void print(int n, v_double& answer) {
	for (int i = 0; i < n; ++i)	{
		std::cout << answer[i] << ' ';
	}
	printf("\n");
}

int main() {
	freopen("input.txt", "r", stdin);

	int n;
	std::cin >> n;

	std::vector<v_double> matrix(n);
	v_double b;

	for (int i = 0; i < n; ++i)	{
		scan(n, matrix[i]);
	}
	scan(n, b);

	std::cout << "n = " << n << std::endl;
	print(n, b);
//	print(n, matrix[499]);
}