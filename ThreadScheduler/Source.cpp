#include "includes/thread_pool_includes.h"

int main()
{
    jpd::ThreadPool Pool(std::thread::hardware_concurrency());

    std::cout << "Testing Modification Of Reference Data" << std::endl;
    std::vector<int> break_test(100000);
    std::mutex safe_mutex;

    auto inner_update_futures = Pool.QueueAndPartitionLoop(0, 1000000, 4, [](size_t a, size_t b, std::vector<int>& value, std::mutex& m)
                                                                     {
                                                                         for (size_t i = a; i < b; ++i)
                                                                         {
                                                                             BEGIN_SCOPE_LOCK(m)
                                                                                value[0] += 1;
                                                                             END_SCOPE_LOCK()
                                                                         }
                                                                         return value[0];
                                                                     }, REF(break_test), REF(safe_mutex));

    // Wait For All Threads To Complete
    for (auto results : inner_update_futures.GetResults())
        std::cout << "Multi-Future Parallelized Loop Results: " << results << std::endl;
    std::cout << "Value: " << break_test[0] << std::endl;
}