#include <coroutine>
#include <functional>
#include <iostream>
#include <thread>

// Promise
template<typename T>
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

    T m_value = -1;
    void return_value(T value)
    {
      m_value = value;
      std::cout << "[promise_type] return_value" << std::endl;
    }
  };
  std::coroutine_handle<promise_type> handle;

  T Value()
  {
    return handle.promise().m_value;
  }
};

// coroutine
Co<int>
run()
{
  std::cout << "[run] before" << std::endl;
  co_return 345;
  std::cout << "[run] after" << std::endl;
}

int
main(int argc, char** argv)
{
  auto co = run();
  std::cout << "[main]before resume: " << co.handle.done() << " => "
            << co.Value() << std::endl;
  co.handle.resume();
  std::cout << "[main]after resume: " << co.handle.done() << " => "
            << co.Value() << std::endl;
  return 0;
}
