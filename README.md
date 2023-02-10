# Thread Pool


## 1. Supported Functionality

### 1.1. Initializing Thread Pool

```c++
explicit ThreadPool( const size_t ThreadCount = 0
                   , const size_t MinimumPartitionSize = 0 ) noexcept;
```

| Params | Details |
| --- | --- |
| ThreadCount | <p>Number of worker threads to be created for the Thread Pool<br>*i.e. The number of worker threads may be modified during runtime using* `ThreadPool::ResetThreads( NewThreadCount )`</p> |
| MinimumPartitionSize | <p>Minimum number of indexes to be processed per thread<br>`**Note: ThreadPool::m_MinPartitionSize has a lower priority than the MinPartitionSize parameter in QueueAndPartitionLoop`</p> |

### 1.2. Changing Number Of Worker Threads

```c++
inline
void ResetThreads(const size_t ThreadCount = 0) noexcept;
```
| Params | Details |
| --- | --- |
| ThreadCount | <p>Number of worker threads to be created for the Thread Pool<br>`**Note: ResetThreads() waits for all active tasks to be completed before re-initializing the Thread Pool`</p> |

### 1.3. Queuing Functions

```c++
template < typename    Func
         , typename... T_Args
         , typename    ReturnType = std::invoke_result_t < std::decay_t<Func>, T_Args...> >
inline [[nodiscard]]
std::future<ReturnType> QueueFunction( Func&&      F
                                     , T_Args&&... Args ) noexcept;
```

| Params | Details |
| --- | --- |
| F | <p>Function parameter passed by reference<br>*i.e. Global Fn, Member Fn, Lambda*</p> |
| Args | <p>Arguments to be passed by copy or reference to function `F`<br>*e.g.* `REF(x)` *to pass a variable* `x` *as* `std::reference_wrapper(x)`</p> |

### 1.4. Queuing Loops

```c++
template < typename    Func
         , typename... T_Args
         , typename    ReturnType = std::invoke_result_t<std::decay_t<Func>, size_t, size_t, T_Args...> >
inline [[nodiscard]]
GroupTasks<ReturnType> QueueAndPartitionLoop( const size_t EndIndex
                                            , const size_t PartitionCount
                                            , const size_t MinPartitionSize
                                            , Func&&       F
                                            , T_Args&&...  Args ) noexcept;


template < typename    Func
         , typename... T_Args
         , typename    ReturnType = std::invoke_result_t<std::decay_t<Func>, size_t, size_t, T_Args...> >
inline [[nodiscard]]
GroupTasks<ReturnType> QueueAndPartitionLoop( const size_t StartIndex
                                            , const size_t EndIndex
                                            , const size_t PartitionCount
                                            , const size_t MinPartitionSize
                                            , Func&&       F
                                            , T_Args&&...  Args ) noexcept;

```
| Params | Details |
| --- | --- |
| StartIndex | <p>Starting index of for loop</p> |
| EndIndex | <p>Ending index of for loop</p> |
| PartitionCount | <p>Number of partitions to sub-divide the for loop into<br>*i.e.* 1 - `std::thread::hardware_concurrency`</p> |
| MinPartitionSize | <p>Minimum number of indexes to be processed per thread<br>*i.e. MinPartitionSize = 5 while handling 20 iterations would result in 4 threads being used regardless of PartitionCount*<br>`**Note: MinPartitionSize takes priority over ThreadPool::m_MinPartitionSize which is initialized in the Constructor`</p> |
| F | <p>Function parameter passed by reference<br>*i.e. Global Fn, Member Fn, Lambda*</p> |
| Args | <p>Arguments to be passed by copy or reference to function `F`<br>*e.g.* `REF(x)` *to pass a variable* `x` *as* `std::reference_wrapper(x)`</p> |


## 2. Generic Function Examples

### 2.1. Global Functions

```c++
int NF_SumIntegers(int sum, int y)
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


int main()
{
    // Construct Thread Pool w/ Max Available Threads
    jpd::ThreadPool Pool( std::thread::hardware_concurrency() );

    {
        // 1. Args Passed By Copy
        auto NF_Sum = Pool.QueueFunction( NF_SumIntegers, 10, 12 );
        // Use .get() to retrieve value from std::future
        std::cout << "Args: " << NF_Sum.get() << std::endl;


        // 2. Args Passed By Ref - Requires Reference Wrapper
        std::mutex MutexLock;
        int x = 100, y = 122;
        auto NF_RefSum = Pool.QueueFunction( NF_REF_SumIntegers, REF(x), REF(y), REF(MutexLock) );
        // Wait for queued function to complete
        NF_RefSum.wait();
        std::cout << "Ref Args: " << x << std::endl;
    }
}
```

### 2.2. Member Functions

```c++
/*
    **Note: The Class Variable MUST BE the FIRST parameter
            to be passed as a reference when using Member
            Functions

    Refer to QueueFunction in main() for example
*/

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
    // Create AdditionClass which contains the relevant Member Function
    AdditionClass AddHelper( 200, 244 );

    // Args
    auto MF_Sum = Pool.QueueFunction( &AdditionClass::MF_SumIntegers, REF(AddHelper), 20, 24 );
    // Use .get() to retrieve value from std::future
    std::cout << "Args: " << MF_Sum.get() << std::endl;

    
    // Ref Args
    auto MF_REF_Sum = Pool.QueueFunction( &AdditionClass::MF_REF_SumIntegers, REF(AddHelper) );
    std::cout << "Ref Args: " << MF_REF_Sum.get()
                              << " vs "
                              << AddHelper.m_X << std::endl;
}
```

### 2.3. Lambdas

```c++
int main()
{
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
    // Wait for queued function to complete
    LAMBDA_REF_Sum.wait();
    std::cout << "\tRef Args: " << x << std::endl;
}
```

### 2.4 For Loops

```c++
int main()
{
    // Ref Args
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
    std::cout << "Ref Args: ";
    for (auto v : LAMBDA_Loop.GetResults())
    {
        std::cout << v << " ";
    }
}
```