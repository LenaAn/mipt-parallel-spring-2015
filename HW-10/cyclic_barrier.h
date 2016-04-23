#ifndef H_CYCLIC_BARRIER
#define H_CYCLIC_BARRIER

#include <mutex>
#include <condition_variable>

class cyclic_barrier {
	size_t arrived;
	size_t passed;
	const size_t total;

	std::mutex access_to_arrived;
	std::mutex access_to_passed;
	std::condition_variable all_arrived;
	std::condition_variable all_passed;

public:
	cyclic_barrier(const size_t _n): total(_n), arrived(0), passed(0) {
	};

	void enter() {
		std::unique_lock<std::mutex> passed_lck(access_to_passed);   //подождать, что все вышли
		while (passed % total != 0) {
			all_passed.wait(passed_lck);
		}
		passed_lck.unlock();

		std::unique_lock<std::mutex> arrived_lck(access_to_arrived);
		arrived++;
	
		if (arrived == total) {										 //если не последний - подождать, иначе - разбудить всех
			all_arrived.notify_all();
		} else while (arrived < total) {
			all_arrived.wait(arrived_lck);
		}
		arrived_lck.unlock();

		passed_lck.lock();
		passed++;													//сказать, что я вышел

		if (passed == total) {										//если вышел последним, сказать об этом всем, обнулить счетчик
			all_passed.notify_all();
			passed = 0;
			arrived_lck.lock();
			arrived = 0;
			arrived_lck.unlock();
		}
	}	
};
#endif