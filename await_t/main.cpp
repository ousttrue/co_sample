#include <asio.hpp>

int
main(int argc, char** argv)
{
  asio::io_context io;

  io.run();

  return 0;
}
