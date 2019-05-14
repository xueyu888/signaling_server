#pragma once
#include "tcp_session.h"
#include "session_delegate.h"
#include <iostream>
int g_tcp_session_num = 0;

tcp_session::tcp_session(tcp::socket socket, 
                         std::shared_ptr<session_delegate> session_dg) :
                         socket_(std::move(socket)),
                         session_delegate_(session_dg),
                         strand_(socket_.get_executor()) {
  socket_.set_option(boost::asio::ip::tcp::no_delay(true));
  printf("%s g_tcp_session_num %d\n", __func__, ++g_tcp_session_num);
}

tcp_session::~tcp_session()
{
  printf("%s g_tcp_session_num %d\n", __func__, --g_tcp_session_num);
}

void tcp_session::run() {
  read();
}

void tcp_session::on_read(const boost::system::error_code& ec,
                             std::shared_ptr<message> buffer) {
  std::string ss(buffer->msg);
  std::string::iterator it = ss.begin();
  while (it != ss.end()) {
    auto it_start = it;
    it = std::find(it, ss.end(), '}');
    if (it == ss.end())
      break;
    it+=2;

    auto msg = std::make_shared<std::string>(it_start, it);
    
    if (session_delegate_)
      session_delegate_->on_read(shared_from_this(), ec, msg);
  }

  if (ec) {
    printf("%s rx error. code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
    close();
    return;
  }

  read();
}

void tcp_session::read() {
  auto buffer = std::make_shared<message> ();
  socket_.async_read_some(boost::asio::buffer((buffer.get())->msg, 3000),
                          boost::asio::bind_executor(
                                strand_,
                                boost::bind(&tcp_session::on_read,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                buffer))); 
}

void tcp_session::on_send(const boost::system::error_code& ec,
                      std::shared_ptr<message> buffer) {
  std::string ss(buffer->msg);
  std::string::iterator it = ss.begin();
  while (it != ss.end()) {
    auto it_start = it;
    it = std::find(it, ss.end(), '}');
    if (it == ss.end())
      break;
    it+=2;

    auto msg = std::make_shared<std::string>(it_start, it);
    
    if (session_delegate_)
      session_delegate_->on_send(shared_from_this(), ec, msg);
  }

  if (ec) {                     
    printf("%s tx error: code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
    close();
    return;
  }
}

void tcp_session::send(std::shared_ptr<std::string> buffer) {
    auto msg = std::make_shared<message> ();
    strcpy_s(msg->msg, buffer->c_str());
    async_write(socket_, boost::asio::buffer(msg->msg, strlen(msg->msg)),
              boost::bind(&tcp_session::on_send,
                          shared_from_this(),
                          boost::asio::placeholders::error,
                          msg));
}

void tcp_session::close() {
  boost::system::error_code ec;
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  socket_.close();
  if (session_delegate_)
    session_delegate_->on_close(shared_from_this());
}

boost::asio::io_context& tcp_session::get_context() {
  return socket_.get_io_context();
}