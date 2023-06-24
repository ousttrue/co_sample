#include <asio.hpp>
#include <asio/co_spawn.hpp>
#include <asio/use_awaitable.hpp>
#include <iostream>

asio::awaitable<void>
co(const char* host, const char* service)
{
  auto executor = co_await asio::this_coro::executor;
  asio::ip::tcp::resolver resolver(executor);
  asio::ip::tcp::resolver::results_type endpoints =
    co_await resolver.async_resolve(host, service, asio::use_awaitable);
  for (auto& p : endpoints) {
    std::cout << p.endpoint() << std::endl;
  }

  asio::ip::tcp::socket socket(executor);
  co_await asio::async_connect(socket, endpoints, asio::use_awaitable);

  asio::streambuf request;
  std::ostream request_stream(&request);
  request_stream << "GET / HTTP/1.0\r\n";
  request_stream << "Host: " << host << "\r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";
  std::cout << "GET..." << std::endl;
  co_await asio::async_write(socket, request, asio::use_awaitable);

  // Check that response is OK.
  asio::streambuf response;
  co_await asio::async_read_until(
    socket, response, "\r\n", asio::use_awaitable);
  {
    if (response.size() > 0) {
      std::cout << &response;
    }
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
      std::cout << "Invalid response\n";
      co_return;
    }
    if (status_code != 200) {
      std::cout << "Response returned with status code ";
      std::cout << status_code << "\n";
      // co_return;
    }
  }

  // Process the response headers.
  asio::async_read_until(socket, response, "\r\n\r\n", asio::use_awaitable);
  std::istream response_stream(&response);
  {
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";
  }
  if (response.size() > 0) {
    std::cout << &response;
  }

  // Start reading remaining data until EOF.
  while (true) {
    co_await asio::async_read(
      socket, response, asio::transfer_at_least(1), asio::use_awaitable);
    std::cout << &response;
  }
}

int
main(int argc, char** argv)
{
  asio::io_context io_context;

  asio::co_spawn(io_context, co("think-async.com", "http"), asio::detached);

  io_context.run();

  return 0;
}
