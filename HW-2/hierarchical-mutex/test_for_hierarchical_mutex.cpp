#include <iostream>
#include <thread>
#include <ctime>
#include <vector>
#include "hierarchical_mutex.h"

int main() {
	hierarchical_mutex mtx1(1), mtx2(2), mtx3(3);

	std::thread t1( [&](){
		std::this_thread::sleep_for( std::chrono::seconds(rand() % 10) );
		mtx3.lock();
		std::this_thread::sleep_for( std::chrono::seconds(rand() % 10) );
		mtx2.lock();
		std::this_thread::sleep_for( std::chrono::seconds(rand() % 10) );
		mtx1.lock();
	} );

	std::thread t2( [&](){
		std::this_thread::sleep_for( std::chrono::seconds(rand() % 10) );
		mtx1.lock();
		std::this_thread::sleep_for( std::chrono::seconds(rand() % 10) );
		mtx2.lock();
		std::this_thread::sleep_for( std::chrono::seconds(rand() % 10) );
		mtx3.lock();
	} );
	

	t1.join();
	t2.join();
	
	return 0;
}