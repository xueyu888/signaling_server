#pragma once

#include "sender.h"

using namespace boost::asio::ip;
using namespace boost::asio;



class tcp_session : public sender,
                    public std::enable_shared_from_this<tcp_session> {
public:  
  tcp_session(tcp::socket socket, std::shared_ptr<session_delegate> session_dg);
  ~tcp_session();
  
  void run();
  void read();
  void on_read(const boost::system::error_code& ec,
                    std::shared_ptr<message> buffer);
         
  void send(std::shared_ptr<std::string> buffer);
  void on_send(const boost::system::error_code& ec,
                    std::shared_ptr<message> buffer);

  void close();
  boost::asio::io_context& get_context();
private:
  tcp::socket socket_;
  std::shared_ptr<session_delegate> session_delegate_;
  boost::asio::strand<
      boost::asio::io_context::executor_type> strand_;
};