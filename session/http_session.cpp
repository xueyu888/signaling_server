#include "http_session.h"
#include "string"



/********************************************************************************
 ** Class http_session
 **/
http_session::http_session(tcp::socket socket, 
                           std::shared_ptr<session_delegate> session_dg) 
        : socket_(std::move(socket))
        , strand_(socket_.get_executor())
        , session_delegate_(session_dg)
{

}

http_session::~http_session() {}

void http_session::run()
{
  printf("The session has been created. socket %d\n", socket_.native_handle());
  read();
}

void http_session::read() 
{
    // Read a request
    http::async_read(socket_, buffer_, req_,
        boost::asio::bind_executor(
            strand_,
            std::bind(
                &http_session::on_read,
                shared_from_this(),
                std::placeholders::_1)));
}

void http_session::on_read(boost::system::error_code ec) 
{
  auto buffer = std::make_shared<std::string> (req_.body());
  if (session_delegate_)
    session_delegate_->on_send(shared_from_this(), ec, buffer);

  if(ec) {
      printf("%s http rx error: code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
      close();
      return;
  }

  // See if it is a WebSocket Upgrade
  if(websocket::is_upgrade(req_)) {
      //TODO
      return;
  }
  read();
}

void http_session::send(std::shared_ptr<std::string> buffer) 
{
    http::response<http::string_body> res;
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(true);
    res.body() = *buffer.get();
    res.prepare_payload();

    write(std::move(res), buffer);
}

template<bool isRequest, class Body, class Fields>
void http_session::write(http::message<isRequest, Body, Fields>&& msg,
                         std::shared_ptr<std::string> buffer) 
{

  // Write the response
  http::async_write(
      socket_, 
      msg,
      boost::asio::bind_executor(
          strand_,
          std::bind(
              &http_session::on_write,
              shared_from_this(),
              std::placeholders::_1,
			  msg.need_eof(),
              buffer)));
}

void http_session::on_write(boost::system::error_code ec, 
                            bool b_close,
                            std::shared_ptr<std::string> buffer)
{
    
    if (session_delegate_ && buffer)
      session_delegate_->on_send(shared_from_this(), ec, buffer);

    if(ec) {
      printf("%s http on_write error: code %d msg %s \n", __func__, ec.value(), ec.message().c_str());
      close();
      return;
    }

    if(b_close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return close();
    }

}

void http_session::close() {
  if (session_delegate_)
    session_delegate_->on_close(shared_from_this());
  socket_.close();
  printf("The session has been closed. socket %d\n", socket_.native_handle());
}

boost::asio::io_context& http_session::get_context() {
  return socket_.get_io_context();
}