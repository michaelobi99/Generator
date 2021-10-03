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
	auto ran = range(1, 4);
	try {
		std::cout << "\n" << ran.next() << "\n";
		std::cout << "\n" << ran.next() << "\n";
		std::cout << "\n" << ran.next() << "\n";
		std::cout << "\n" << ran.next() << "\n";
	}
	catch (std::exception const& error) {
		std::cout << error.what() << "\n";
	}
	for (auto elem : range(1, 200, 2)) {
		std::cout << elem << " ";
	}
}