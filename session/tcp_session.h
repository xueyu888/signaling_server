#pragma once

#include "session.h"

using namespace boost::asio::ip;
using namespace boost::asio;

class tcp_session : public session,
                    public std::enable_shared_from_this<session> {
public:  
  tcp_session(tcp::socket socket, std::shared_ptr<session_delegate> session_dg);
  ~tcp_session() = default;

  void run() {read();}
  void read();
  void on_read(const boost::system::error_code& ec,
                    std::shared_ptr<std::string> buffer);
         
  void send(std::shared_ptr<std::string> buffer);
  void on_send(const boost::system::error_code& ec,
                    std::shared_ptr<std::string> buffer);

  void set_protocol(std::string& protocol);
  void close();
  boost::asio::io_context& get_context();
private:
  tcp::socket socket_;
  std::string protocol_;
  std::shared_ptr<session_delegate> session_delegate_;
};