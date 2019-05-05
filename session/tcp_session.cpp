#pragma once
#include "tcp_session.h"
#include "session_delegate.h"
#include <iostream>

tcp_session::tcp_session(tcp::socket socket, 
                         std::shared_ptr<session_delegate> session_dg) :
                         socket_(std::move(socket)),
                         session_delegate_(session_dg) {
	
}

void tcp_session::run() {
  printf("The session has been created. socket %d\n", socket_.native_handle());
  read();
}

void tcp_session::on_read(const boost::system::error_code& ec,
                             std::shared_ptr<std::string> buffer) {
  if (session_delegate_)
    session_delegate_->on_read(shared_from_this(), ec, buffer);

  if (ec) {
    printf("%s rx error. code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
    if (session_delegate_)
    close();
    return;
  }

  read();
}

void tcp_session::read() {
  auto buffer = std::make_shared<std::string> ();
  async_read(socket_, boost::asio::buffer(*buffer.get()),
              boost::bind(&tcp_session::on_read,
                          shared_from_this(),
                          boost::asio::placeholders::error,
                          buffer));
}

void tcp_session::on_send(const boost::system::error_code& ec,
                      std::shared_ptr<std::string> buffer) {
  if (session_delegate_)
    session_delegate_->on_send(shared_from_this(), ec, buffer);

  if (ec) {                     
    printf("%s tx error: code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
    close();
    return;
  }
}

void tcp_session::send(std::shared_ptr<std::string> buffer) {
    async_write(socket_, boost::asio::buffer(*buffer.get()),
              boost::bind(&tcp_session::on_send,
                          shared_from_this(),
                          boost::asio::placeholders::error,
                          buffer));
}

void tcp_session::close() {
  if (session_delegate_)
    session_delegate_->on_close(shared_from_this());
  socket_.close();
  printf("The session has been closed. socket %d\n", socket_.native_handle());
}

boost::asio::io_context& tcp_session::get_context() {
  return socket_.get_io_context();
}