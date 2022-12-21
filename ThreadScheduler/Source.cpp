#include "includes/thread_pool_includes.h"

int func(int sleep_time, std::string s)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    std::cout << s << std::endl;
    return 69;
}

int main()
{
    //// Fake Lambda For Testing
    //auto Return69 = [] { return 69; };
    //// Create Future
    //jpd::GroupTasks<int> GroupedFutures(0);
    //// Create Promise & Assign Task
    //std::shared_ptr<std::promise<int>> f1 = std::make_shared<std::promise<int>>();
    //f1->set_value( Return69() );
    //// Insert Task
    //GroupedFutures.InsertFuture(f1->get_future());
    //// Wait For All Tasks
    //GroupedFutures.WaitForAll();
    //// Get Results
    //auto results = GroupedFutures.GetResults();
    //// Print Results
    //std::cout << "Future Result Test:" << std::endl;
    //for (auto res : results)
    //    std::cout << res << std::endl;
    //std::cout << "\n\n\n";



    //// Create Test Vector
    //std::vector<int> TestVectorContainer(69, 1);
    //// Create Partition Helper w/ __ Threads
    //jpd::PartitionTasks PartitionClass( std::thread::hardware_concurrency() );
    //std::cout << "Max Available Threads: " << std::thread::hardware_concurrency() << std::endl;
    //// Partition
    //auto part_results = PartitionClass.PartitionData(TestVectorContainer);
    //std::cout << "Partition Result Test:" << std::endl;
    //// Print Results
    //for (auto res : part_results)
    //    std::cout << res << " | ";
    //// Print Results
    //std::cout << "\nComparing Split Sums: " << std::accumulate(part_results.begin(), part_results.end(), 0) << " vs " << TestVectorContainer.size() << std::endl;
    //assert(std::accumulate(part_results.begin(), part_results.end(), 0) == TestVectorContainer.size());
    //std::cout << "\n\n\n";

    //

    jpd::ThreadPool Pool(std::thread::hardware_concurrency() - 1); // Use Only 3 Threads For E.g.
    //auto future1 = Pool.QueueFunction(func, 0500, "Testing da bomb 1");
    //auto future2 = Pool.QueueFunction(func, 1000, "Testing da bomb 2");
    //auto future3 = Pool.QueueFunction(func, 1500, "Testing da bomb 3");
    //Pool.QueueFunction(func, 2500, "Testing da bomb 4");
    //Pool.QueueFunction(func, 4500, "Testing da bomb 5");
    //Pool.QueueFunction(func, 1500, "Testing da bomb 6");
    //Pool.QueueFunction(func, 500, "Testing da bomb 7");
    //Pool.QueueFunction(func, 2500, "Testing da bomb 8");
    //Pool.WaitForAllTasks();
    ////std::cout << future1.get() << std::endl;
    ////std::cout << future2.get() << std::endl;
    ////std::cout << future3.get() << std::endl;
    //
    //Pool.ResetThreads();

    //std::cout << "\n\n\nDisplaying all Multi-Future Values" << std::endl;
    //auto multi_futures = Pool.QueueAndPartitionLoop(0, 15, 3, [](size_t a, size_t b, int value)
    //                                                          {
    //                                                              for (size_t i = a; i < b; ++i)
    //                                                              {
    //                                                                  std::cout << "Parallelized Loop Value: " << i << value << std::endl;
    //                                                              }
    //                                                              return value;
    //                                                          }, 69);
    //auto multi_future_results = multi_futures.GetResults();
    //for (auto results : multi_future_results)
    //    std::cout << "Multi-Future Parallelized Loop Results: " << results << std::endl;





    std::cout << "\n\n\nTesting Modification Of Reference Data" << std::endl;
    std::vector<int> break_test(100000);
    std::mutex mmm;

    auto inner_update_futures = Pool.QueueAndPartitionLoop(0, break_test.size(), 64, [](size_t a, size_t b, std::vector<int>& value, std::mutex& m)
                                                                     {
                                                                         for (size_t i = a; i < b; ++i)
                                                                         {
                                                                             ScopeLock(m);
                                                                             value[0] += 1;
                                                                         }
                                                                         return value[0];
                                                                     }, Ref(break_test), Ref(mmm));
    for (size_t i = 0, max = break_test.size(); i < max; ++i)
    {
        ScopeLock(mmm);
        break_test[0] += 1;
    }
    // Wait For All Threads To Complete
    for (auto results : inner_update_futures.GetResults())
        std::cout << "Multi-Future Parallelized Loop Results: " << results << std::endl;
    std::cout << "Value: " << break_test[0] << std::endl;
}