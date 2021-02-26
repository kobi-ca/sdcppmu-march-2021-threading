#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include "fmt/format.h"

struct Foo final {
   int a_{};
};

int main() {
    std::thread t1([] {});
    t1.join(); // must this one

    std::thread t2([] { std::clog << fmt::format("in t2\n"); });
    t2.detach(); // detached t2, no need to call t2.join();

    Foo f1;
    std::thread t3([&f1] { std::clog << fmt::format("in t3 f.a_ {}\n", f1.a_); });
    t3.join();

    Foo f2{7};
    std::thread t4([] (const Foo& f) { std::clog << fmt::format("in t4 f.a_ {}\n", f.a_); }, f2);
    t4.join();

    Foo f3{7};
    const auto func = [] (Foo& f) { f.a_ = 42; std::clog << fmt::format("in t5 f.a_ {}\n", f.a_); };
    std::thread t5(func, std::ref(f3)); // must to have this cref since it is passed by value
    // https://stackoverflow.com/questions/62289941/pass-const-reference-to-thread
    // https://stackoverflow.com/questions/34469490/passing-non-const-reference-to-a-thread
    t5.join(); // must have join and not detached
    std::clog << fmt::format("after t5 f.a_ is {}\n", f3.a_);

    // packaged_task
    const auto f = [](const Foo& foo) { return foo.a_; };
    std::packaged_task<int (const Foo&)> task(f);
    std::future<int> result = task.get_future();
    Foo f4{f3};
    std::thread t6(std::move(task), f4); // need to move it
    t6.join();
    std::clog << fmt::format("future got {}\n", result.get());

    return 0;
}
