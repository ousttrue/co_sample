#include <coroutine>
#include <functional>
#include <iostream>
#include <thread>

struct Awaiter
{
  bool await_ready()
  {
    std::cout << "[Awaiter] await_ready" << std::endl;
    return false;
  }
  void await_suspend(std::coroutine_handle<> handle)
  {
    std::cout << "[Awaiter] await_suspend" << std::endl;
  }
  void await_resume() { std::cout << "[Awaiter] await_resume" << std::endl; }
};

// Promise
struct Co
{
  struct promise_type
  {
    auto get_return_object()
    {
      return Co{ std::coroutine_handle<promise_type>::from_promise(*this) };
    }
    auto initial_suspend()
    {
      std::cout << "[promise_type] initial_suspend" << std::endl;
      return std::suspend_always{};
    }
    auto final_suspend() noexcept
    {
      std::cout << "[promise_type] final_suspend" << std::endl;
      return std::suspend_always{};
    }
    void unhandled_exception() { std::terminate(); }
    void return_void()
    {
      std::cout << "[promise_type] return_void" << std::endl;
    }

    int m_value = -1;
#if 1
    std::suspend_always yield_value(int value)
    {
      m_value = value;
      return {};
    }
#else
    std::suspend_always await_transform(int value)
    {
      m_value = value;
      return {};
    }
#endif
  };
  std::coroutine_handle<promise_type> handle;
};

// coroutine
Co
run()
{
  std::cout << "[run] before" << std::endl;
  co_yield 1;
  co_yield 2;
  co_yield 3;
  std::cout << "[run] after" << std::endl;
  // co_return;
}

int
main(int argc, char** argv)
{
  auto co = run();
  for (int i = 0; !co.handle.done(); ++i) {
    std::cout << "[main]resume " << i << "=>" << co.handle.promise().m_value
              << std::endl;
    co.handle.resume();
  }
  return 0;
}
