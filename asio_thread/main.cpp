#include <asio.hpp>
#include <asio/use_awaitable.hpp>
#include <iostream>
#include <thread>

// https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp20/operations/callback_wrapper.cpp
template<typename T>
struct ThreadTask
{
  template<typename Callback>
  static void Execute(const std::function<T()>& task, Callback cb)
  {
    std::thread([task, cb = std::move(cb)]() mutable {
      std::move(cb)(task());
    }).detach();
  }

  template<asio::completion_token_for<void(T)> CompletionToken>
  static auto AsyncThredTask(const std::function<T()>& task,
                             CompletionToken token)
  {
    auto init = [task](asio::completion_handler_for<void(T)> auto handler) {
      auto work = asio::make_work_guard(handler);

      Execute(task,
              [handler = std::move(handler),
               work = std::move(work)](T result) mutable {
                // Get the handler's associated allocator. If the handler
                // does not specify an allocator, use the recycling
                // allocator as the default.
                auto alloc = asio::get_associated_allocator(
                  handler, asio::recycling_allocator<void>());

                // Dispatch the completion handler through the handler's
                // associated executor, using the handler's associated
                // allocator.
                asio::post(
                  work.get_executor(),
                  asio::bind_allocator(
                    alloc, [handler = std::move(handler), result]() mutable {
                      std::move(handler)(result);
                    }));
              });
    };

    return asio::async_initiate<CompletionToken, void(T)>(init, token);
  }
};

template<typename F>
decltype(auto)
Launch(F f)
{
  using R = decltype(f());
  return ThreadTask<R>::AsyncThredTask(f, asio::use_awaitable);
}

asio::awaitable<void>
task()
{
  // auto io = co_await asio::this_coro::executor;
  auto n = co_await Launch([]() {
    std::cout << "[onThread] " << std::this_thread::get_id() << std::endl;
    return 456;
  });
  std::cout << "[co_await] " << std::this_thread::get_id() << ": " << n
            << std::endl;
}

int
main(int argc, char** argv)
{
  asio::io_context io;

  asio::co_spawn(io, task(), asio::detached);

  std::cout << "[main] run..." << std::endl;
  io.run();
  std::cout << "[main] done" << std::endl;

  return 0;
}
