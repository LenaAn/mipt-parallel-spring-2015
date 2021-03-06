#include <iostream>
#include <atomic>
#include <thread>
#include <future>
#include <cstdint>
#include <random>

#include "barrier.h"
#include "steady_timer.h"

#include "lock_free_stack.h"

//////////////////////////////////////////////////////////////////////////

using int_t = uint64_t;
using ts_stack_t = lock_free_stack<int_t>;

//////////////////////////////////////////////////////////////////////////

void push_to_stack(ts_stack_t& stack, const int_t e) {
    stack.push(e);
}

bool try_pop(ts_stack_t& stack, int_t& e) {
    //return stack.pop(e);

    auto top_ptr = stack.pop();
    if (!!top_ptr) {
        e = *top_ptr;
        return true;
    } else {
        return false;
    }

}

int_t pop_while_not_empty(ts_stack_t& stack) {
    int_t sum = 0;
    int_t e;
    while (try_pop(stack, e)) {
        sum += e;
    }
    return sum;
}

//////////////////////////////////////////////////////////////////////////

void pause(int_t e) {
    // 1) no pause
    return;

    // 2) busy loop
    volatile size_t step = 0;
    while (step * step < e) {
        ++step;
    }
}

void perf_test(const size_t num_producers, const size_t num_elements_to_produce, const size_t num_consumers) {
    std::cout << "start perf test..." << std::endl;

    ts_stack_t stack;

    barrier start_barrier(num_producers + num_consumers);
    std::atomic_size_t producers_done_cnt(0);

    steady_timer timer;

    std::vector<std::future<int_t>> producers;
    for (size_t i = 0; i < num_producers; ++i) {
        auto producer = [&]() {
            start_barrier.enter();

            std::random_device random_dev;
            std::mt19937 mt_random_engine(random_dev());
            std::uniform_int_distribution<int_t> gen_random_int(1, 1000000);

            int_t produced_sum = 0;
            for (size_t k = 0; k < num_elements_to_produce; ++k) {
                auto e = gen_random_int(mt_random_engine);
                push_to_stack(stack, e);
                produced_sum += e;
                pause(e);
            }

            producers_done_cnt.fetch_add(1);

            return produced_sum;
        };

        producers.push_back(std::async(producer));
    }

    std::vector<std::future<int_t>> consumers;
    for (size_t i = 0; i < num_consumers; ++i) {
        auto consumer = [&]() {
            start_barrier.enter();

            int_t consumed_sum = 0;

            do {
                consumed_sum += pop_while_not_empty(stack);
                std::this_thread::yield();
            } while (producers_done_cnt != num_producers);
            consumed_sum += pop_while_not_empty(stack);

            return consumed_sum;
        };

        consumers.push_back(std::async(consumer));
    }

    int_t produced_sum = 0;
    for (auto& f : producers) {
        produced_sum += f.get();
    }

    int_t consumed_sum = 0;
    for (auto& f : consumers) {
        consumed_sum += f.get();
    }

    double elapsed = timer.seconds_elapsed();

    std::cout << "perf test: num producers = " << num_producers << ", num_consumers = " << num_consumers << std::endl;
    std::cout << "produced sum = " << produced_sum << std::endl;
    std::cout << "consumed sum = " << consumed_sum << std::endl;
    std::cout << "result: " << ((produced_sum == consumed_sum) ? "SUCCEEDED" : "FAILED") << std::endl;
    std::cout << "test takes " << elapsed << " seconds" << std::endl;
}

//////////////////////////////////////////////////////////////////////////

int main() {
    perf_test(1, 100000, 1);
    perf_test(5, 100000, 5);
    perf_test(10, 100000, 10);
    perf_test(20, 100000, 20);
    return 0;
}
