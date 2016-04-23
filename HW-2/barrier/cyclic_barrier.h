#ifndef H_CYCLIC_BARRIER
#define H_CYCLIC_BARRIER

#include <mutex>
#include <condition_variable>

class cyclic_barrier {
	size_t arrived;
	size_t passed;
	const size_t total;

	std::mutex access_to_count;
	std::condition_variable all_arrived;
	std::condition_variable all_passed;

public:
	cyclic_barrier(const size_t _n): total(_n), arrived(0), passed(0) {
	};

	void enter() {
		std::unique_lock<std::mutex> count_lck(access_to_count);   //подождать, что все вышли
		while (passed % total != 0) {
			all_passed.wait(count_lck);
		}

		arrived++;
	
		if (arrived == total) {										 //если не последний - подождать, иначе - разбудить всех
			all_arrived.notify_all();
		} else while (arrived < total) {
			all_arrived.wait(count_lck);
		}

		passed++;													//сказать, что я вышел

		if (passed == total) {										//если вышел последним, сказать об этом всем, обнулить счетчик
			all_passed.notify_all();
			passed = 0;
			arrived = 0;
		}
	}	
};
#endif