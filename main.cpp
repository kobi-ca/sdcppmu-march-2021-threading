#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include "fmt/format.h"

namespace {

    void home_alone() {
        std::string s{"stack, please dont leave me behind :("};
        using namespace std::chrono_literals;
        std::thread t ([&s] { std::this_thread::sleep_for(3s); s+= "!"; std::clog << s << '\n';});
        t.detach(); // ouch... s is left behind
    }

    void do_race_condition() {
        std::string race_me{"hello world"};
        std::thread t1([&race_me] { race_me = "goodbye world"; });
        std::thread t2([&race_me] { race_me += "!"; });
        t1.join();
        t2.join();
        std::clog << fmt::format("{}\n", race_me);
        // you'll get all kinds of combinations including:
        // "goodbye worl"
    }

    std::exception_ptr ep = nullptr;
    void let_me_handle_your_error(){
        std::thread([] {
            try {
                throw std::runtime_error("error");
            } catch (...) {
                ep = std::current_exception();
            }
        }).join();
        if (ep) {
            try {
                std::rethrow_exception(ep);
            } catch(std::runtime_error& e) {
                std::cout << e.what() << "\n";
            }
        }
    }

    void packaged_task_will_throw() {
        auto pt = std::packaged_task<void()>([]{ throw std::runtime_error("Error!"); });
        auto f = pt.get_future();
        auto t = std::thread(std::move(pt));
        t.join();
        try {
            f.get();
        } catch (const std::runtime_error& ex) {
            std::clog << fmt::format("packaged_task_will_throw {}\n", ex.what());
        }
    }

    void join_a_detached_thread() {
        std::thread t {[] { std::clog << "going to join a detached thread\n"; }};
        t.detach();
        t.join(); // will abort
    }

    void using_reference_as_comm_method() {
        auto data = std::optional<std::string>{std::nullopt};
        // producer thread
        auto t = std::thread([&] {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(500ms);
            data = "42";
        });
        t.join();
        // consumer expression
        std::clog << *data << '\n';
    }

    void producer_consumer() {
        auto promise = std::promise<std::string>{};
        auto future = promise.get_future();
        // producer thread
        auto t = std::thread([](auto promise) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(500ms);
            promise.set_value("42");
        }, std::move(promise));
        // consumer expression
        std::clog << future.get() << '\n';
        t.join();
    }

    // repeat after me, I shall not use volatile as if it was atomic
    volatile bool done = false;

    // this is UB...
    void using_volatile_as_bool() {
        std::thread t1([] {
            while(!done) {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1s);
            }
        });

        std::thread t2([] {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(2s);
            done = true;
        });
        t1.join(); t2.join();
        std::clog << "finished using_volatile_as_bool\n";
    }

    struct Foo final {
        int a_{};
    };

} // namespace anonymous


int main() {
    std::thread t1([] {});
    t1.join(); // must this one

    std::thread t2([] { std::clog << fmt::format("in t2\n"); });
    t2.detach(); // detached t2, no need to call t2.join();

    Foo f1;
    std::thread t3([&f1] { std::clog << fmt::format("in t3 f.a_ {}\n", f1.a_); });
    t3.join();

    Foo f2{7};
    std::thread t4([](const Foo &f) { std::clog << fmt::format("in t4 f.a_ {}\n", f.a_); }, f2);
    t4.join();

    Foo f3{7};
    const auto func = [](Foo &f) {
        f.a_ = 42;
        std::clog << fmt::format("in t5 f.a_ {}\n", f.a_);
    };
    std::thread t5(func, std::ref(f3)); // must to have this cref since it is passed by value
    // https://stackoverflow.com/questions/62289941/pass-const-reference-to-thread
    // https://stackoverflow.com/questions/34469490/passing-non-const-reference-to-a-thread
    t5.join(); // must have join and not detached
    std::clog << fmt::format("after t5 f.a_ is {}\n", f3.a_);

    // packaged_task
    const auto f = [](const Foo &foo) { return foo.a_; };
    std::packaged_task<int(const Foo &)> task(f);
    std::future<int> result = task.get_future();
    Foo f4{f3};
    std::thread t6(std::move(task), f4); // need to move it
    t6.join();
    std::clog << fmt::format("future got {}\n", result.get());

    // async
    auto fut = std::async(f, f4);
    std::clog << fmt::format("async future got {}\n", fut.get());

//    do_race_condition();
//    home_alone();
//    let_me_handle_your_error();
//    packaged_task_will_throw();
//    join_a_detached_thread();
//    using_reference_as_comm_method();
//    producer_consumer();
    using_volatile_as_bool();

    return 0;
}
