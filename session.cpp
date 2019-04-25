#include "session.h"
#include "middle.h"
#include <iostream>

session::session(tcp::socket socket) :
                      socket_(std::move(socket)) {
                    
}

void session::on_read(const boost::system::error_code& ec,
                             std::shared_ptr<std::string> buffer) {
  if (ec) {
    printf("%s rx error. code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
    close();
    return;
  }

  handle_request(shared_from_this(), buffer);

  async_read(socket_, boost::asio::buffer(*buffer.get()),
              boost::bind(&session::on_read, 
                          shared_from_this(),
                          boost::asio::placeholders::error,
                          buffer));
}

void session::read() {
  std::shared_ptr<std::string> buffer;
  async_read(socket_, boost::asio::buffer(*buffer.get()),
              boost::bind(&session::on_read, 
                          shared_from_this(),
                          boost::asio::placeholders::error,
                          buffer));
}

void session::on_send(const boost::system::error_code& ec,
                      std::shared_ptr<std::string> buffer) {
  if (ec) {                     
    printf("%s tx error: code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
    close();
    return;
  }
}

void session::send(std::shared_ptr<std::string> buffer) {
    async_write(socket_, boost::asio::buffer(*buffer.get()),
              boost::bind(&session::on_send, 
                          shared_from_this(),
                          boost::asio::placeholders::error,
                          buffer));
}

void session::set_protocol(std::string& protocol) {
  protocol_ = protocol;
}

boost::asio::io_context& session::get_context() {
  return socket_.get_io_context();
}

void session::close() {
  handle_close(shared_from_this());
  socket_.close();
}