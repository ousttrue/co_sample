#include <coroutine>
#include <functional>
#include <iostream>
#include <thread>

// Promise
struct Co {
  struct promise_type {
    auto get_return_object() {
      return Co{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    auto initial_suspend() {
      std::cout << "[promise_type] initial_suspend" << std::endl;
      return std::suspend_always{};
    }
    auto final_suspend() noexcept {
      std::cout << "[promise_type] final_suspend" << std::endl;
      return std::suspend_always{};
    }
    void unhandled_exception() { std::terminate(); }
    void return_void() {}
  };
  std::coroutine_handle<promise_type> handle;
};

// coroutine
Co run() {
  std::cout << "[run] before" << std::endl;
  co_return;
  std::cout << "[run] after" << std::endl;
}

int main(int argc, char **argv) {

  auto co = run();
  std::cout << "[main]before resume: " << co.handle.done() << std::endl;
  co.handle.resume();
  std::cout << "[main]after resume: " << co.handle.done() << std::endl;
  return 0;
}
