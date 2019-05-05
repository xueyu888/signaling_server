#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "sender.h"
#include "session_delegate.h"

using namespace boost::asio::ip;
using namespace boost::beast;

// Handles an HTTP server connection
class http_session : public std::enable_shared_from_this<http_session>, 
                     public sender
{
public:
  // Take ownership of the socket
  explicit
  http_session(tcp::socket socket,
               std::shared_ptr<session_delegate> session_dg);
  ~http_session();
  
  template<bool isRequest, class Body, class Fields>
  void write(http::message<isRequest, Body, Fields>&& msg, 
             std::shared_ptr<std::string> buffer);
  void on_write(boost::system::error_code ec, 
                bool close, 
                std::shared_ptr<std::string> buffer);
  // Start the asynchronous operation
  void run();

  void read();
  void on_read(boost::system::error_code ec);

  void send(std::shared_ptr<std::string> buffer);
  void on_send(boost::system::error_code ec, bool close);
  
  void close();
  boost::asio::io_context& get_context();

private:
  tcp::socket socket_;
  boost::asio::strand<
      boost::asio::io_context::executor_type> strand_;
  boost::beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
  std::shared_ptr<void> res_;
  std::shared_ptr<session_delegate> session_delegate_;
};
