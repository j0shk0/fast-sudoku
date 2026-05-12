#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "Sudoku.h"

namespace beast = boost::beast;    // from <boost/beast.hpp>
namespace http = beast::http;      // from <boost/beast/http.hpp>
namespace net = boost::asio;       // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request) is type-erased in message_generator.
template <class Body, class Allocator>
static http::message_generator
handle_request(  // NOLINT(*-use-anonymous-namespace)
    http::message<true, Body, http::basic_fields<Allocator>>&& req) {
  // Returns a bad request response
  auto const bad_request = [&req](const beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Returns a server error response
  auto const server_error = [&req](beast::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method
  if (req.method() != http::verb::get && req.method() != http::verb::head)
    return bad_request("Unknown HTTP-method");

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos)
    return bad_request("Illegal request-target");

  // Default route returns a valid sudoku board.
  if (req.target() == "/") {
    http::response<http::string_body> res{http::status::ok, req.version()};
    Sudoku tmp_instance{};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    tmp_instance.generate();
    res.body() = tmp_instance.getBoardString();
    res.prepare_payload();
    return res;
  };

  // A request that includes "board=" will return a response with the validity
  // of the board, while the integers behind the board= are the entries of the
  // board.
  if (req.target().contains("board=")) {
    const std::string target{req.target()};
    const std::size_t pos = target.find("board=");
    const std::size_t begin = pos + 6;  // length of "board="
    const std::size_t end = target.find('&', begin);
    std::string board = target.substr(
        begin, end == std::string::npos ? std::string::npos : end - begin);

    // Validate length
    if (board.size() != 81) {
      return bad_request("Board must be exactly 81 digits");
    }

    // Validate all characters are digits 1-9
    if (!std::ranges::all_of(
            board, [](const char c) { return c >= '1' && c <= '9'; })) {
      return bad_request("Board must contain only digits 1-9");
    }

    std::array<int, 81> solution{};
    std::ranges::transform(board.begin(), board.end(), solution.begin(),
                           [](const char c) -> int { return (c - '0'); });

    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    Sudoku tmp_instance(solution.data());
    res.body() = std::to_string(tmp_instance.check());
    res.prepare_payload();
    return res;
  }

  return server_error("Unknown request");
}

//------------------------------------------------------------------------------

// Report a failure
static void fail( // NOLINT(*-use-anonymous-namespace)
    const beast::error_code& ec,  // NOLINT(*-use-anonymous-namespace)
    char const* what) {           // NOLINT(*-use-anonymous-namespace)
  std::cerr << what << ": " << ec.message() << "\n";
}

// Handles an HTTP server connection
class Session : public std::enable_shared_from_this<Session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_{1024};
  boost::optional<http::request_parser<http::string_body>> parser_;

 public:
  // Take ownership of the stream
  explicit Session(tcp::socket&& socket) : stream_(std::move(socket)) {}

  // Start the asynchronous operation
  void run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&Session::do_read, shared_from_this()));
  }

  void do_read() {
    parser_.emplace();
    parser_->body_limit(0);

    stream_.expires_after(std::chrono::seconds(30));

    http::async_read(
        stream_, buffer_, *parser_,
        beast::bind_front_handler(&Session::on_read, shared_from_this()));
  }

  void on_read(const beast::error_code& ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream) return do_close();

    if (ec) return fail(ec, "read");

    send_response(handle_request(parser_->release()));
  }

  void send_response(http::message_generator&& msg) {
    bool keep_alive = msg.keep_alive();

    // Write the response
    beast::async_write(stream_, std::move(msg),
                       beast::bind_front_handler(
                           &Session::on_write, shared_from_this(), keep_alive));
  }

  void on_write(const bool keep_alive, const beast::error_code& ec,
                std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "write");

    if (!keep_alive) {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return do_close();
    }

    // Read another request
    do_read();
  }

  void do_close() {
    // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
  }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class Listener : public std::enable_shared_from_this<Listener> {
  net::io_context& ioc_;
  tcp::acceptor acceptor_;

 public:
  Listener(net::io_context& ioc, const tcp::endpoint& endpoint)
      : ioc_(ioc), acceptor_(net::make_strand(ioc)) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  void run() { do_accept(); }

 private:
  void do_accept() {
    // The new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
  }

  void on_accept(const beast::error_code& ec, tcp::socket socket) {
    if (ec) {
      fail(ec, "accept");
      return;  // To avoid infinite loop
    }  // Create the session and run it
    std::make_shared<Session>(std::move(socket))->run();

    // Accept another connection
    do_accept();
  }
};

//------------------------------------------------------------------------------

int main(const int argc, char* argv[]) {
  // Check command line arguments.
  if (argc != 4) {
    std::cerr << "Usage: http-server-async <address> <port> <threads>\n"
              << "Example:\n"
              << "    http-server-async 0.0.0.0 8080 1\n";
    return EXIT_FAILURE;
  }

  boost::system::error_code ec;
  auto const address = net::ip::make_address(argv[1], ec);
  if (ec) {
    std::cerr << "Invalid address: " << ec.message() << "\n";
    return EXIT_FAILURE;
  }

  char* end = nullptr;
  auto const port_long = std::strtol(argv[2], &end, 10);
  if (*end != '\0' || port_long <= 0 || port_long > 65535) {
    std::cerr << "Invalid port (must be 1-65535)\n";
    return EXIT_FAILURE;
  }
  auto const port = static_cast<uint16_t>(port_long);

  auto const threads_long = std::strtol(argv[3], &end, 10);
  if (*end != '\0' || threads_long <= 0 || threads_long > 256) {
    std::cerr << "Invalid thread count (must be 1-256)\n";
    return EXIT_FAILURE;
  }
  auto const threads = static_cast<int>(threads_long);

  // The io_context is required for all I/O
  net::io_context ioc{threads};

  // Create and launch a listening port
  std::make_shared<Listener>(ioc, tcp::endpoint{address, port})->run();

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i) v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();

  return EXIT_SUCCESS;
}
