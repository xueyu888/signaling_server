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
  socket_.set_option(boost::asio::ip::tcp::no_delay(true));
}

http_session::~http_session() {}

void http_session::run()
{
  printf("The session has been created. socket %d\n", socket_.native_handle());
  read();
}

void http_session::read() 
{
  auto req = std::make_shared<http::request<http::string_body>>();
  // Read a request
  http::async_read(socket_, buffer_, *req,
      boost::asio::bind_executor(
          strand_,
          std::bind(
              &http_session::on_read,
              shared_from_this(),
              std::placeholders::_1,
              req)));
}

void http_session::on_read(boost::system::error_code ec,
                           std::shared_ptr<http::request<http::string_body>> req) 
{
  auto buffer = std::make_shared<std::string> (req->body());
  if (session_delegate_)
    session_delegate_->on_read(shared_from_this(), ec, buffer);

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
    auto res = std::make_shared<http::response<http::string_body>>();
    res->set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res->set(http::field::content_type, "text/html");
    res->keep_alive(true);
    res->body() = *buffer.get();
    res->prepare_payload();

    write(res);
}

void http_session::write(std::shared_ptr<http::response<http::string_body>> msg) 
{
  // Write the response
  http::async_write(
      socket_, 
      *msg,
      boost::asio::bind_executor(
          strand_,
          std::bind(
              &http_session::on_write,
              shared_from_this(),
			        std::placeholders::_1,
              //std::placeholders::_2,
              msg,
              msg->need_eof())));
}

void http_session::on_write(
                            boost::system::error_code ec,
                            //std::size_t bytes_transferred,
                            std::shared_ptr<http::response<http::string_body>> msg,
                            bool b_close)
{
	auto buffer = std::make_shared<std::string>(msg->body());
    if (session_delegate_)
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
  socket_.close();
  if (session_delegate_)
    session_delegate_->on_close(shared_from_this());
  
  printf("The session has been closed. socket %d\n", socket_.native_handle());
}

boost::asio::io_context& http_session::get_context() {
  return socket_.get_io_context();
}