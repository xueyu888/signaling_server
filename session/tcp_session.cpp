#include "tcp_session.h"
#include "session_delegate.h"
#include <iostream>
#include <string.h>
#pragma warning(disable : 4996)

int g_tcp_session_num = 0;

tcp_session::tcp_session(tcp::socket socket, 
                         std::shared_ptr<session_delegate> session_dg) :
                         socket_(std::move(socket)),
                         session_delegate_(session_dg),
                         strand_(socket_.get_executor()) {
  socket_.set_option(boost::asio::ip::tcp::no_delay(true));
  memset(cache_data_, 0x0, sizeof(cache_data_));
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
  char* p_cache = cache_data_;
  char* p_start = NULL;
  int cache_len = strlen(cache_data_);

  if (p_cache[0] != 0) {
    memcpy(p_cache + cache_len, buffer->msg, strlen(buffer->msg));
    p_start = p_cache;
  } else {
    p_start = buffer->msg;
  }

  char* data_start = p_start;
  while(*p_start != 0) {
    
    if (*p_start == '}') {
      auto msg = std::make_shared<std::string>(data_start, p_start - data_start + 1);
      if (session_delegate_)
        session_delegate_->on_read(shared_from_this(), ec, msg);

      data_start = p_start + 2;
    }
    p_start++;
  }
  if (*data_start != 0) {
    char* p_next = cache_data_;
    int cat_len = p_start - data_start;
    memcpy(p_next, data_start, cat_len);
    memset(p_next + cat_len, 0x0, sizeof(cache_data_ - cat_len));
  } else {
	if (cache_data_[0] != 0)
	  memset(cache_data_, 0x0, sizeof(cache_data_));
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
  socket_.async_read_some(boost::asio::buffer((buffer.get())->msg, sizeof(buffer->msg)),
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
    strcpy(msg->msg, buffer->c_str());
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
  return socket_.get_executor().context();
}
