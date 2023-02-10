#include "includes/thread_pool_includes.h"

// normal fn
// normal fn with args
// normal fn with reference args (mutex)
// lambda fn
// lambda fn with args
// lambda fn with reference args (mutex)
// member fn
// member fn with args
// member fn with reference args (mutex)
// loop no args
// loop with args
// loop with reference args (mutex)t


// Group -> GetResults = Get Values
// Group -> WaitForAll = Just Wait

int NF_Void()
{
    return 100;
}

int NF_SumIntegers(int sum = 0, int y = 0)
{
    sum += y;
    return sum;
}

void NF_REF_SumIntegers(int& sum, int& y, std::mutex& m)
{
    // Lock Member Variables
    BEGIN_SCOPE_LOCK(m);
        sum += y;
    END_SCOPE_LOCK();
}

struct AdditionClass
{
    AdditionClass(int x, int y) : m_X{ x }, m_Y{ y } { }
    ~AdditionClass() = default;

    int MF_SumIntegers(int sum = 0, int y = 0)
    {
        sum += y;
        return sum;
    }

    int MF_REF_SumIntegers()
    {
        // Lock Member Variables
        BEGIN_SCOPE_LOCK(m_Lock);
            m_X += m_Y;
        END_SCOPE_LOCK();
        return m_X;
    }

    std::mutex m_Lock;
    int m_X = 0;
    int m_Y = 0;
};


int main()
{
    // Construct Thread Pool w/ Max Available Threads
    jpd::ThreadPool Pool( std::thread::hardware_concurrency() );


    // Case 0: Normal Function
    std::cout << "Case 0: Normal Function" << std::endl;
    {
        // No Args
        auto NF_RetVal = Pool.QueueFunction( NF_Void );
        std::cout << "\tNo Args: " << NF_RetVal.get() << std::endl;


        // Args
        auto NF_Sum = Pool.QueueFunction( NF_SumIntegers, 10, 12 );
        std::cout << "\tArgs: " << NF_Sum.get() << std::endl;


        // Ref Args - Requires Reference Wrapper
        std::mutex MutexLock;
        int x = 100, y = 122;
        auto NF_RefSum = Pool.QueueFunction( NF_REF_SumIntegers, REF(x), REF(y), REF(MutexLock) );
        NF_RefSum.wait();
        std::cout << "\tRef Args: " << x << std::endl;
    }


    // Case 1: Lambda Function
    std::cout << "Case 1: Lambda Function" << std::endl;
    {
        // No Args
        auto LAMBDA_RetVal = Pool.QueueFunction( []() -> int
                                                 {
                                                     int f_x = 100, f_y = 100;
                                                     return f_x + f_y;
                                                 });
        std::cout << "\tNo Args: " << LAMBDA_RetVal.get() << std::endl;


        // Args
        auto LAMBDA_Sum = Pool.QueueFunction( [&](int sum, int y = 0)
                                              {
                                                  sum += y;
                                                  return sum;
                                              }, 10, 23);
        std::cout << "\tArgs: " << LAMBDA_Sum.get() << std::endl;


        // Ref Args
        std::mutex MutexLock;
        int x = 100, y = 233;
        auto LAMBDA_REF_Sum = Pool.QueueFunction( [](int& sum, int& y, std::mutex& m)
                                                  {
                                                      // Lock Member Variables
                                                      BEGIN_SCOPE_LOCK(m);
                                                          sum += y;
                                                      END_SCOPE_LOCK();
                                                  }, REF(x), REF(y), REF(MutexLock) );
        LAMBDA_REF_Sum.wait();
        std::cout << "\tRef Args: " << x << std::endl;
    }


    // Case 2: Member Function
    std::cout << "Case 2: Member Function" << std::endl;
    {
        // Args
        AdditionClass AddHelper(200, 244);
        auto MF_Sum = Pool.QueueFunction(&AdditionClass::MF_SumIntegers, REF(AddHelper), 20, 24);
        std::cout << "\tArgs: " << MF_Sum.get() << std::endl;

        
        // Ref Args
        auto MF_REF_Sum = Pool.QueueFunction(&AdditionClass::MF_REF_SumIntegers, REF(AddHelper));
        std::cout << "\tRef Args: " << MF_REF_Sum.get() << " vs " << AddHelper.m_X << std::endl;
    }


    // Case 3: Loop
    std::cout << "Case 3: Loop" << std::endl;
    {
        // Args
        std::mutex MutexLock;
        std::vector<int> SumArray(10, 1);
        auto LAMBDA_Loop = Pool.QueueAndPartitionLoop(0, 10, 16, 0, [](size_t a, size_t b, std::vector<int>& value, std::mutex& m)
            {
                                                                         for (size_t i = a; i < b; ++i)
                                                                         {
                                                                             if (i > 0)
                                                                             {
                                                                                 BEGIN_SCOPE_LOCK(m)
                                                                                     value[i] = value[i - 1] * 2;
                                                                                 END_SCOPE_LOCK()
                                                                             }
                                                                         }
                                                                         return value[a];
                                                                     }, REF(SumArray), REF(MutexLock));
        //LAMBDA_Loop.WaitForAll();
        std::cout << "\tRef Args: ";
        for (auto v : LAMBDA_Loop.GetResults())
        {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }









    //std::cout << "Testing Modification Of Reference Data" << std::endl;
    //std::vector<int> break_test(100000);
    //std::mutex safe_mutex;

    //auto inner_update_futures = Pool.QueueAndPartitionLoop(0, 125, 16, 0, [](size_t a, size_t b, std::vector<int>& value, std::mutex& m)
    //                                                                     {
    //                                                                         for (size_t i = a; i < b; ++i)
    //                                                                         {
    //                                                                             BEGIN_SCOPE_LOCK(m)
    //                                                                                value[0] += 1;
    //                                                                             END_SCOPE_LOCK()
    //                                                                         }
    //                                                                         return value[0];
    //                                                                     }, REF(break_test), REF(safe_mutex));

    //// Wait For All Threads To Complete
    //for (auto results : inner_update_futures.GetResults())
    //    std::cout << "Multi-Future Parallelized Loop Results: " << results << std::endl;
    //std::cout << "Value: " << break_test[0] << std::endl;
}