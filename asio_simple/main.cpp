#include <asio.hpp>
#include <asio/use_awaitable.hpp>
#include <iostream>

asio::awaitable<int>
async_get()
{
  co_return 1;
}

asio::awaitable<void>
task()
{
  // auto io = co_await asio::this_coro::executor;
  auto n = co_await async_get();
  std::cout << n << std::endl;
}

int
main(int argc, char** argv)
{
  asio::io_context io;

  asio::co_spawn(io, task(), asio::detached);

  std::cout << "run..." << std::endl;
  io.run();
  std::cout << "done" << std::endl;

  return 0;
}
