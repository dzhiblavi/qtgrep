#include "gtest/gtest.h"

#include <iostream>
#include <unistd.h>
#include <set>
#include <filesystem>
#include <fstream>

#include "thread_pool.h"
#include "fault_injection/fault_injection.hpp"
#include "task.h"
#include "grep_task.h"

#define WAIT usleep(25000)

std::mutex m;
std::set<int> executing;
std::vector<int> cancelled;

void clear() {
    fault_injection_disable fd;
    executing.clear();
    cancelled.clear();
}

struct test_task : task {
    int id;
    std::atomic_bool cancel_;

    bool is_cancelled() {
        return this->tp_.is_aborted() || cancel_.load();
    }

    void cancel() {
        cancel_.store(true);
    }

    test_task(int id, thread_pool& tp)
        : task(tp)
        , id(id)
        , cancel_(false)
    {
#ifdef CONSTR_LOG
        std::cerr << "TASK " << id << " CONSTRUCTED\n";
#endif
    }

    ~test_task() {
#ifdef DESTR_LOG
        std::cerr << "TASK " << id << " DESTROYED\n";
#endif
    }

    void prepare() noexcept override {
        cancel_.store(false);
    }

    void run() noexcept override {
        {
            fault_injection_disable fi;
            std::unique_lock<std::mutex> lg(m);
            executing.insert(id);
        }
        for (;;) {
            if (is_cancelled()) {
                {
                    fault_injection_disable di;
                    std::unique_lock<std::mutex> lg(m);
                    cancelled.push_back(id);
                }
                break;
            }
        }
        {
            fault_injection_disable di;
            std::unique_lock<std::mutex> lg(m);
            executing.erase(id);
        }
    }
};

TEST(correctness, super_simple) {
    faulty_run([]
    {
        clear();

        thread_pool tp(3);
        for (size_t i = 0; i < 10; ++i) {
            std::shared_ptr<test_task> tsk;
            {
                fault_injection_disable di;
                tsk = std::make_shared<test_task>(i, tp);
            }
            tp.enqueue(tsk);
        }
        WAIT;
        EXPECT_EQ(executing.size(), 3);
        EXPECT_EQ(tp.queue_size(), 10 - 3);
        tp.abort();
        WAIT;
        EXPECT_EQ(executing.size(), 0);
        EXPECT_EQ(tp.queue_size(), 0);
        EXPECT_EQ(cancelled.size(), 3);
    });
}

TEST(correctness, simple) {
//    thread_pool tp(3);
//    std::vector<std::shared_ptr<test_task>> tasks;
//    for (size_t i = 0; i < 10; ++i) {
//        tasks.emplace_back(std::make_shared<test_task>(i, tp));
//    }
//
//    faulty_run([&]
//    {
//        clear();
//
//        tp.abort();
//        WAIT;
//        EXPECT_EQ(executing.size(), 0);
//        EXPECT_EQ(tp.queue_size(), 0);
//        EXPECT_EQ(cancelled.size(), 3);
//
//        for (size_t i = 0; i < 10; ++i) {
//            tp.enqueue(tasks[i]);
//        }
//
//        WAIT;
//        EXPECT_EQ(executing.size(), 3);
//        EXPECT_EQ(tp.queue_size(), 10 - 3);
//    });
}

TEST(correctness, cancel_and_reuse) {
    faulty_run([]
    {
        clear();

        thread_pool tp(3);
        std::vector<std::shared_ptr<test_task>> tasks;
        for (int i = 0; i < 3; ++i) {
            std::shared_ptr<test_task> tsk;
            {
                fault_injection_disable di;
                tsk = std::make_shared<test_task>(i, tp);
            }
            tasks.emplace_back(tsk);
        }

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 3; ++j) {
                tp.enqueue(tasks[j]);
            }

            WAIT;
            EXPECT_EQ(0, tp.queue_size());
            EXPECT_EQ(3, executing.size());

            for (int j = 0; j < 3; ++j) {
                tasks[j]->cancel();

                WAIT;
                EXPECT_EQ(0, tp.queue_size());
                EXPECT_EQ(3 - j - 1, executing.size());
                EXPECT_EQ(j, cancelled.back());
            }

            tp.abort();

            WAIT;
            EXPECT_EQ(0, tp.queue_size());
            EXPECT_EQ(0, executing.size());
        }
    });
}

TEST(correctness, load_100K) {
    clear();
    thread_pool tp(100);
    for (size_t i = 0; i < 100000; ++i) {
        std::shared_ptr<test_task> tsk;
        {
            fault_injection_disable di;
            tsk = std::make_shared<test_task>(i, tp);
        }
        tp.enqueue(tsk);
    }
    WAIT;
    EXPECT_EQ(100000 - 100, tp.queue_size());
    EXPECT_EQ(100, executing.size());
    tp.abort();
    WAIT;
    EXPECT_EQ(0, tp.queue_size());
    EXPECT_EQ(0, executing.size());
    EXPECT_EQ(100, cancelled.size());
}

TEST(grep_background, empty_dir) {
    faulty_run([]
    {
        thread_pool tp(2);
        auto gtask = std::make_shared<grep_task>("/Users/dzhiblavi/Documents"
                                                 "/prog/cpp/code/qtgrep"
                                                 "/test/empty_dir", "abacaba", tp);
        tp.enqueue(gtask);
        WAIT;
        auto v = gtask->get_result();
        for (auto const& x : v) {
            std::cout << x.toStdString();
        }
    });
}

TEST(grep_background, file) {
    faulty_run([]
    {
        thread_pool tp(2);
        auto gtask = std::make_shared<grep_task>("/Users/dzhiblavi/Documents"
                                                 "/prog/cpp/code/qtgrep"
                                                 "/test/", "abacaba", tp);
        tp.enqueue(gtask);
        WAIT;
        auto v = gtask->get_result();
        for (auto const& x : v) {
            std::cout << x.toStdString();
        }
    });
}

TEST(grep_background, large_dir_long_lines) {
    thread_pool tp(2);
    auto gtask = std::make_shared<grep_task>("/Users/dzhiblavi/Documents", "int", tp);
    tp.enqueue(gtask);
    std::ofstream ofile("root_dir.test.txt");
    size_t cnt = 0;
    while (!gtask->finished()) {
        WAIT;
        auto v = gtask->get_result();
        gtask->clear_result();
        for (auto const& x : v) {
            ofile << x.toStdString();
        }
        if (cnt++ > 3) {
            break;
        }
    }
    gtask->cancel();
}

TEST(grep_background, empty_substr) {
    thread_pool tp(2);
    auto gtask = std::make_shared<grep_task>("/Users/dzhiblavi/Documents"
                                             "/prog/cpp/code/qtgrep"
                                             "/test/", "", tp);
    tp.enqueue(gtask);
    WAIT;
    auto v = gtask->get_result();
    for (auto const& x : v) {
        std::cout << x.toStdString();
    }
}
