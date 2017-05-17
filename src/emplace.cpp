#include <gtest/gtest.h>
#include <vector>
#include <chrono>
using namespace std;

struct A{
	int x;
	double y;
	A(int a, double b):x(a), y(b){}
};

TEST(emplace, push_back)
{
	vector<A> v;

	auto start = std::chrono::high_resolution_clock::now();

	for(size_t i = 0; i < 1000000; ++i ) {
		v.push_back(A(1, 2.0));
	}

	auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast< std::chrono::duration< double >>( end - start );
    std::cout << "seconds = " << delta.count() << std::endl;
}

TEST(emplace, emplace_back)
{
	vector<A> v;

	auto start = std::chrono::high_resolution_clock::now();

	for(size_t i = 0; i < 1000000; ++i ) {
		v.emplace_back(1, 2.0);
	}

	auto end = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast< std::chrono::duration< double >>( end - start );
    std::cout << "seconds = " << delta.count() << std::endl;
}
