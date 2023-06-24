#include <asio.hpp>
#include <asio/use_awaitable.hpp>
#include <iostream>

asio::awaitable<int>
async_get_123()
{
  co_return 123;
}

asio::awaitable<int>
async_get_nest()
{
  auto value = co_await async_get_123();
  co_return value;
}

asio::awaitable<void>
task()
{
  // auto io = co_await asio::this_coro::executor;
  auto n = co_await async_get_nest();
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
