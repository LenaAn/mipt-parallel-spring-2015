#include <atomic>
#include <thread>

class TAS_spinlock {
	std::atomic<bool> locked;
public:
	TAS_spinlock(): locked(false) {
	}
	void lock() {
		while ( locked.exchange(true, std::memory_order_acquire) ) {
			std::this_thread::yield();
		}
	}

	void unlock() {
		locked.store(false, std::memory_order_release);
	}

	bool try_lock() {
		return !locked.exchange(true, std::memory_order_acquire);
	}
};