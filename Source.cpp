#include "Generator.h"
#include <iostream>
Generator<int> range(int start = 0, int stop = std::numeric_limits<int>::max(), int step = 1) {
	int value = start;
	while (true) {
		co_yield value;
		value += step;
		if (value >= stop)
			break;
	}
}
int main() {
	for (auto elem : range(1, 200, 5))
		std::cout << elem << " ";
	std::cout << "\n" << std::numeric_limits<int>::max();
}