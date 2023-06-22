#include <chrono>
#include <coroutine>
#include <functional>
#include <future>
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

    auto await_transform(const std::function<T()>& work)
    {
      struct awaiter
      {
        std::future<T> m_future;
        std::thread m_t;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<>) {}
        T await_resume() // 👈 await_resume の返り値が co_await  の左辺値になる
        {
          std::cout << "thread block ? tid#" << std::this_thread::get_id()
                    << std::endl;
          return m_future.get();
        }
      };

      std::promise<T> p;
      std::future<T> f = p.get_future();
      std::thread t([work, p = std::move(p)]() mutable {
        std::cout << "[work]tid#" << std::this_thread::get_id() << std::endl;
        T value = work();   // 👈 thread 上で work を実行
        p.set_value(value); // 結果を返す
      });

      // join しないのでエラーにならなように
      t.detach();

      return awaiter{
        std::move(f),
        std::move(t),
      };
    }

    T m_value = -1;
    void return_value(T value)
    {
      m_value = value;
      std::cout << "[promise_type] return_value" << std::endl;
    }
  };
  std::coroutine_handle<promise_type> handle;

  T Value() { return handle.promise().m_value; }
};

// coroutine
Co<int>
run()
{
  std::cout << "[run] before" << std::endl;
  int result = co_await []() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 678;
  };
  std::cout << "[run] after" << std::endl;
  co_return result;
}

int
main(int argc, char** argv)
{
  auto co = run();
  while (!co.handle.done()) {
    co.handle.resume();
  }
  std::cout << "[main]" << co.Value() << std::endl;
  return 0;
}
