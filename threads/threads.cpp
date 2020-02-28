/*
*/
#include <algorithm>
#include <mango/mango.hpp>

using namespace mango;

static inline
void print(const char* text)
{
    printf("%s", text);
    fflush(stdout);
}

void test0()
{
    ConcurrentQueue q;

    std::atomic<int> counter { 0 };

    constexpr u64 icount = 7'500'000;

    for (u64 i = 0; i < icount; ++i)
    {
        q.enqueue([&]
        {
            ++counter;
        });
    }
}

void test1()
{
    ConcurrentQueue cq;
    SerialQueue sa;
    SerialQueue sb;

    std::atomic<int> counter { 0 };

    constexpr u64 icount = 1000;

    for (u64 i = 0; i < icount; ++i)
    {
        cq.enqueue([&]
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            print(".");

            sa.enqueue([&]
            {
                ++counter;
                std::this_thread::sleep_for(std::chrono::microseconds(7));
                print("1");
            });

            sb.enqueue([&]
            {
                ++counter;
                std::this_thread::sleep_for(std::chrono::microseconds(2));
                print("2");
            });

#if 1
            if (i % 8 == 0)
            {
                sa.wait();
                sb.wait();
            }
#endif
        });
    }

    u64 time0 = Time::us();

    FutureTask<int> xtask([] () -> int {
        return 7;
    });

    int x = xtask.get();

    u64 time1 = Time::us();

    print("x");

    cq.wait(); // producer must be synchronized first
    print("A");

    sa.wait();
    print("B");

    sb.wait();
    print("C");

    u64 time2 = Time::us();

    FutureTask<int> ytask([] () -> int
    {
        return 9;
    });

    int y = ytask.get();

    u64 time3 = Time::us();

    printf("\n\n");
    printf("counter: %d [%s]\n", counter.load(), counter == icount * 2 ? "Success" : "FAILED");
    printf("load latency: %d us\n", int(time1 - time0));
    printf("idle latency: %d us\n", int(time3 - time2));
}

void test2()
{
    ConcurrentQueue q;

    u64 time0 = Time::ms();

    std::atomic<int> counter { 0 };

    constexpr  u64 icount = 1000;
    constexpr  u64 jcount = 1000;

    for (u64 i = 0; i < icount; ++i)
    {
        q.enqueue([&]
        {
            for (int j = 0; j < jcount; ++j)
            {
                q.enqueue([&]
                {
                    ++counter;
                });
            }
        });
    }

    u64 time1 = Time::ms();

    printf("enqueue counter: %d\n", counter.load());

    q.wait();

    u64 time2 = Time::ms();

    printf("counter: %d [%s]\n", counter.load(), counter == icount * jcount ? "Success" : "FAILED");
    printf("enqueue: %d ms, execute: %d ms\n", int(time1 - time0), int(time2 - time1));
}

void test3()
{
    SerialQueue q;

    u64 time0 = Time::ms();

    std::atomic<int> counter { 0 };

    constexpr u64 icount = 1000;
    constexpr u64 jcount = 1000;

    for (u64 i = 0; i < icount; ++i)
    {
        //print(".");

        q.enqueue([&]
        {
            for (int j = 0; j < jcount; ++j)
            {
                //print("+");

                q.enqueue([&]
                {
                    ++counter;
                });
            }
        });
    }

    //q.cancel();

    u64 time1 = Time::ms();

    printf("enqueue counter: %d\n", counter.load());

    q.wait();

    u64 time2 = Time::ms();

    printf("counter: %d [%s]\n", counter.load(), counter == icount * jcount ? "Success" : "FAILED");
    printf("enqueue: %d ms, execute: %d ms\n", int(time1 - time0), int(time2 - time1));
}

void test4()
{
    ConcurrentQueue cq;
    SerialQueue sq;

    std::atomic<int> counter { 0 };
    std::atomic<bool> running { false };

    std::atomic<int> overlaps { 0 };

    std::vector<int> result;
    std::mutex mutex;

    for (u64 i = 0; i < 10; ++i)
    {
        cq.enqueue([&]
        {
            for (int j = 0; j < 10; ++j)
            {
                sq.enqueue([&]
                {
                    if (running.load())
                    {
                        overlaps ++;
                    }

                    running = true;
                    ++counter;

                    //std::lock_guard<std::mutex> lock(mutex);
                    result.push_back(counter.load() - 1);

                    running = false;
                });
            }
        });
    }

    cq.wait();
    sq.wait();

    int errors = 0;
    for (int i = 0; i < result.size(); ++i)
    {
        if (i != result[i])
            errors ++;
    }
    printf("Sequential errors: %d\n", errors);
    printf("Overlapping tasks: %d\n", overlaps.load());

    if (errors > 0 || overlaps.load() > 0)
    {
        printf("Status: ERROR \n");
        exit(1);
    }
}

void test5()
{
    ConcurrentQueue q;

    std::atomic<int> counter { 0 };

    constexpr u64 icount = 1'000'000 / 10;
    constexpr u64 jcount = 10;

    for (u64 i = 0; i < icount; ++i)
    {
        q.enqueue([&]
        {
            for (int j = 0; j < jcount; ++j)
            {
                q.enqueue([&]
                {
                    ++counter;
                });
            }
        });
    }

    q.wait();

    printf("counter: %d [%s]\n", counter.load(), counter == icount * jcount ? "Success" : "FAILED");
}

void test6()
{
    std::atomic<int> acquire_counter { 0 };
    std::atomic<int> consume_counter { 0 };

    ConcurrentQueue q;
    TicketQueue tk;

    constexpr u64 icount = 1'000'000 / 10;

    for (int i = 0; i < icount; ++i)
    {
        auto ticket = tk.acquire();
        ++acquire_counter;

        q.enqueue([ticket, &consume_counter]
        {
            ticket.consume([&consume_counter]
            {
                ++consume_counter;
            });
        });
    }

    q.wait();
    tk.wait();

    printf("  acquire: %d\n", acquire_counter.load());
    printf("  consume: %d\n", consume_counter.load());
}

int main(int argc, char* argv[])
{
    int count = 1;
    if (argc == 2)
    {
        count = std::atoi(argv[1]);
    }

    using Function = void (*)(void);

    Function tests [] =
    {
        test0,
        test1,
        test2,
        test3,
        test4,
        test5,
        test6,
    };

    for (int i = 0; i < count; ++i)
    {
        int index = 0;
        for (auto func : tests)
        {
            printf("------------------------------------------------------------\n");
            printf(" test%d\n", ++index);
            printf("------------------------------------------------------------\n");
            printf("\n");

            u64 time0 = Time::ms();
            func();

            printf("\n <<<< complete: %d ms \n\n", int(Time::ms() - time0));
        }
    }
}
